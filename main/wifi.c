/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

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



static const char *TAG = "ESP32 NAT router";


void state_wifi_deconnected() {

    wifi_init_config_t _wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&_wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_scan_config_t scan_config = {
      .ssid = 0,
      .bssid = 0,
      .channel = 0,
          .show_hidden = true
      };

    ESP_LOGI(TAG,"Start scanning...");
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_LOGI(TAG," completed!\n");
    ESP_LOGI(TAG,"\n");

    // get the list of APs found in the last scan
    const int MAX_AP = 40;
    uint16_t ap_num = MAX_AP;
    wifi_ap_record_t ap_records[MAX_AP];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

    char preferedNetwork[] = SFR_WIFI_SSID;

    const char* targetNetwork = NULL;

    for(int i = 0; i < ap_num; i++) {

      if(targetNetwork == NULL && strcmp((const char*)ap_records[i].ssid, SFR_WIFI_SSID) == 0) {
          targetNetwork = SFR_WIFI_SSID;
      }

      if(targetNetwork == NULL && strcmp((const char*)ap_records[i].ssid, FREE_WIFI_SSID) == 0) {
          targetNetwork = FREE_WIFI_SSID;
      }

        if(preferedNetwork != NULL && strcmp((const char*)ap_records[i].ssid, preferedNetwork) == 0) {
            targetNetwork = preferedNetwork;
        }

    }

    if(targetNetwork == NULL) {
      // void
    }
    else if(strcmp(targetNetwork, SFR_WIFI_SSID) == 0) {
       state = STATE_SFR_WIFI_CONNECT;
    }
    else if(strcmp(targetNetwork, FREE_WIFI_SSID) == 0) {
      // state = STATE_FREE_WIFI_CONNECT;
    }

    ESP_LOGI(TAG,"Target network %s", targetNetwork);

}


void state_wifi_connecting() {

}
