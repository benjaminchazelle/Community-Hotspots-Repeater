/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_wifi.h"


#include "main.h"
#include "config.h"
#include "states.h"
#include "wifi.h"
#include "sfr_wifi.h"
#include "free_wifi.h"



static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}



void app_main(void)
{
    state = STATE_SFR_WIFI_CONNECT;

    initialize_nvs();

    while(true) {

      switch(state) {

        case STATE_WIFI_DECONNECTED:
            state_wifi_deconnected();
            break;

        case STATE_SFR_WIFI_CONNECT:
            state_sfr_wifi_connect();
            break;

        case STATE_SFR_WIFI_CONNECTING:
            state_sfr_wifi_connecting();
            break;

        case STATE_SFR_WIFI_CONNECTED:
            state_sfr_wifi_connected();
            break;

        case STATE_SFR_WIFI_AUTH_FETCHING:
            state_sfr_wifi_auth_fetching();
            break;

        case STATE_SFR_WIFI_AUTH_FETCHED:
            state_sfr_wifi_auth_fetched();
            break;

        case STATE_SFR_WIFI_AUTHENTICATING:
            state_sfr_wifi_authenticating();
            break;

        case STATE_SFR_WIFI_AUTHENTICATED:
            state_sfr_wifi_authenticated();
            break;

        case STATE_FREE_WIFI_CONNECTING:
            state_free_wifi_connecting();
            break;

        case STATE_FREE_WIFI_CONNECTED:
            state_free_wifi_connected();
            break;

        case STATE_FREE_WIFI_AUTHENTICATING:
            state_free_wifi_authenticating();
            break;

        case STATE_FREE_WIFI_AUTHENTICATED:
            state_free_wifi_authenticated();
            break;

      }

      vTaskDelay(3000 / portTICK_RATE_MS);
    }

}
