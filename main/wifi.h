/* Declarations of command registration functions.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/event_groups.h"

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

void state_wifi_deconnected();
void state_wifi_connecting();

#ifdef __cplusplus
}
#endif
