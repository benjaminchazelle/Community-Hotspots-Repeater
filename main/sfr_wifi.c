#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_http_client.h"

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "cmd_decl.h"
#include "router_globals.h"

#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif

#include "config.h"
#include "states.h"


/* ESP WIFI CONFIG */
wifi_config_t wifi_config = {
    .sta = {
      .ssid = SFR_WIFI_SSID,
      .password = "",
    }

 };

    wifi_config_t ap_config = {
      .sta = {
        .ssid = HOTSPOT_SSID
        // , .password = "qdqsdqsd"
      },
    .ap = {
        .channel = 0,
        .authmode = WIFI_AUTH_OPEN, // WIFI_AUTH_WPA2_PSK
        .ssid_hidden = 0,
        .max_connection = 8,
        .beacon_interval = 100,
    }
};

static const char *TAG = "ESP32 NAT router";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;


/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */


uint16_t connect_count = 0;
bool ap_connect = false;


static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG,"start - try to connect to the AP");
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ap_connect = true;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, BIT0);
        if(state == STATE_SFR_WIFI_CONNECTING) {
          state = STATE_SFR_WIFI_CONNECTED;
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG,"disconnected - retry to connect to the AP");
        ap_connect = false;
        state = STATE_WIFI_DECONNECTED;
        xEventGroupClearBits(wifi_event_group, BIT0);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        connect_count++;
        ESP_LOGI(TAG,"%d. station connected", connect_count);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        connect_count--;
        ESP_LOGI(TAG,"station disconnected - %d remain", connect_count);
        break;
    default:
        ESP_LOGI(TAG,"wifi_event_handler - unknown event");
        break;
  }
  return ESP_OK;
}



void wifi_apsta_init(const char* ssid, const char* passwd, const char* ap_ssid, const char* ap_passwd, void* ctx)
{
    ESP_LOGI(TAG, "wifi_apsta_init::begin");
    ip_addr_t dnsserver;
    //tcpip_adapter_dns_info_t dnsinfo;

    wifi_event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "wifi_apsta_init::netif");

    esp_netif_init();
    ESP_LOGI(TAG, "wifi_apsta_init::evnetLoop");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "wifi_apsta_init::esp_netif_create_default_wifi_ap");
    esp_netif_t* wifiAP = esp_netif_create_default_wifi_ap();
    ESP_LOGI(TAG, "wifi_apsta_init::esp_netif_create_default_wifi_sta");
    esp_netif_t* wifiSTA = esp_netif_create_default_wifi_sta();
    ESP_LOGI(TAG, "wifi_apsta_init::ipAddr");

    esp_netif_ip_info_t ipInfo;

    IP4_ADDR(&ipInfo.ip, 192,168,4,1);
    IP4_ADDR(&ipInfo.gw, 192,168,4,1);
    IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
    esp_netif_dhcps_stop(wifiAP); // stop before setting ip WifiAP
    esp_netif_set_ip_info(wifiAP, &ipInfo);
    esp_netif_dhcps_start(wifiAP);
    ESP_LOGI(TAG, "wifi_apsta_init::loopInit");

    ESP_ERROR_CHECK(esp_event_loop_init(ctx, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* ESP WIFI CONFIG */
    wifi_config_t wifi_config = { 0 };
        wifi_config_t ap_config = {
        .ap = {
            .channel = 0,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 8,
            .beacon_interval = 100,
        }
    };
    ESP_LOGI(TAG, "wifi_apsta_init::apConfigCopy");

    strlcpy((char*)ap_config.sta.ssid, ap_ssid, sizeof(ap_config.sta.ssid));
    if (strlen(ap_passwd) < 8) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
	    strlcpy((char*)ap_config.sta.password, ap_passwd, sizeof(ap_config.sta.password));
    }
    ESP_LOGI(TAG, "wifi_apsta_init::wifiConfigCopy");

    if (strlen(ssid) > 0) {
        strlcpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strlcpy((char*)wifi_config.sta.password, passwd, sizeof(wifi_config.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config) );
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config) );
    }
    ESP_LOGI(TAG, "wifi_apsta_init::optionInfo");

    // Enable DNS (offer) for dhcp server
    dhcps_offer_t dhcps_dns_value = OFFER_DNS;
    dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));
    ESP_LOGI(TAG, "wifi_apsta_init::setServer");

    // Set custom dns server address for dhcp server
    dnsserver.u_addr.ip4.addr = htonl(DNS_IP_ADDR);
    dnsserver.type = IPADDR_TYPE_V4;
    dhcps_dns_setserver(&dnsserver);

//    tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_AP, TCPIP_ADAPTER_DNS_MAIN, &dnsinfo);
//    ESP_LOGI(TAG, "DNS IP:" IPSTR, IP2STR(&dnsinfo.ip.u_addr.ip4));
ESP_LOGI(TAG, "wifi_apsta_init::wifiStart");

    xEventGroupWaitBits(wifi_event_group, BIT0,
        pdFALSE, pdTRUE, JOIN_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(esp_wifi_start());

    if (strlen(ssid) > 0) {
        ESP_LOGI(TAG, "wifi_init_apsta finished.");
        ESP_LOGI(TAG, "connect to ap SSID: %s ", ssid);
    } else {
        ESP_LOGI(TAG, "wifi_init_ap with default finished.");
    }

    #if IP_NAPT
    ESP_LOGI(TAG, "wifi_apsta_init::NAT");

    u32_t napt_netif_ip = 0xC0A80401; // Set to ip address of softAP netif (Default is 192.168.4.1)
    ip_napt_enable(htonl(napt_netif_ip), 1);
    ESP_LOGI(TAG, "NAT is enabled");
    #endif
}


void state_sfr_wifi_connect() {
  ESP_LOGI(TAG,"state_sfr_wifi_connect");


  wifi_apsta_init(SFR_WIFI_SSID, "", HOTSPOT_SSID, HOTSPOT_PASSWORD, wifi_event_handler);

  state = STATE_SFR_WIFI_CONNECTING;

}

void state_sfr_wifi_connecting() {
  ESP_LOGI(TAG,"state_sfr_wifi_connecting");

}

char auth_location[350];

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            if(state == STATE_SFR_WIFI_AUTH_FETCHING && strcmp(evt->header_key, "Location") == 0) {
              strcpy(auth_location, evt->header_value);
              state = STATE_SFR_WIFI_AUTH_FETCHED;
              ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER Location" );
            }

             break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            if(state == STATE_SFR_WIFI_AUTH_FETCHING) {
              state = STATE_SFR_WIFI_CONNECTED;
            }
            break;
    }
    return ESP_OK;
}

void state_sfr_wifi_connected() {
  ESP_LOGI(TAG,"state_sfr_wifi_connected");

  state = STATE_SFR_WIFI_AUTH_FETCHING;


  esp_http_client_config_t config = {
     .url = "http://detectportal.firefox.com/success.txt",
     .event_handler = _http_event_handle,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK) {
     ESP_LOGI(TAG, "Status = %d, content_length = %d",
             esp_http_client_get_status_code(client),
             esp_http_client_get_content_length(client));
  }
  esp_http_client_cleanup(client);

}

void state_sfr_wifi_auth_fetching() {
  ESP_LOGI(TAG,"state_sfr_wifi_auth_fetching");

}

void state_sfr_wifi_auth_fetched() {
  ESP_LOGI(TAG,"state_sfr_wifi_auth_fetched %s", auth_location);


  char* uamip = NULL;
  char* uamport = NULL;
  char* challenge = NULL;
  char* userurl = NULL;
  char* nasid = NULL;
  char* mac = NULL;
  char* mode = NULL;
  char* channel = NULL;

  strtok(auth_location, "?" );
  char* queryString = strtok (NULL, "?");

  char* keyValue;
  while(NULL != (keyValue = strtok (queryString, "&"))) {
      queryString = NULL;
      char* key = strsep(&keyValue, "=");
      char* value = keyValue;
      // ESP_LOGI(TAG, "%s => %s\n", key, value);

      if(strcmp(key, "uamip") == 0) {
        uamip = value;
      }

      if(strcmp(key, "uamport") == 0) {
        uamport = value;
      }

      if(strcmp(key, "challenge") == 0) {
        challenge = value;
      }

      if(strcmp(key, "userurl") == 0) {
        userurl = value;
      }

      if(strcmp(key, "nasid") == 0) {
        nasid = value;
      }

      if(strcmp(key, "mac") == 0) {
        mac = value;
      }

      if(strcmp(key, "mode") == 0) {
        mode = value;
      }

      if(strcmp(key, "channel") == 0) {
        channel = value;
      }
  }


  if(uamip != NULL && uamport != NULL && challenge != NULL && userurl != NULL && nasid != NULL && mac != NULL && mode != NULL && channel != NULL) {

    ESP_LOGI(TAG, "EVERYTHING IS AWESOME");

    state = STATE_SFR_WIFI_AUTHENTICATING;

  } else {
    state = STATE_SFR_WIFI_CONNECTED;
  }



}

void state_sfr_wifi_authenticating() {

}

void state_sfr_wifi_authenticated() {

}
