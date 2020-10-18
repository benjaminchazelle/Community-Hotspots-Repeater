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

typedef int STATE;

STATE state;

#define STATE_WIFI_DECONNECTED 0

#define STATE_SFR_WIFI_CONNECT 1
#define STATE_SFR_WIFI_CONNECTING 2
#define STATE_SFR_WIFI_CONNECTED 3
#define STATE_SFR_WIFI_AUTH_FETCHING 4
#define STATE_SFR_WIFI_AUTH_FETCHED 5
#define STATE_SFR_WIFI_AUTHENTICATING 6
#define STATE_SFR_WIFI_AUTHENTICATED 7

#define STATE_FREE_WIFI_CONNECT 8
#define STATE_FREE_WIFI_CONNECTING 9
#define STATE_FREE_WIFI_CONNECTED 10
#define STATE_FREE_WIFI_AUTH_FETCHING 11
#define STATE_FREE_WIFI_AUTH_FETCHED 12
#define STATE_FREE_WIFI_AUTHENTICATING 13
#define STATE_FREE_WIFI_AUTHENTICATED 14


#ifdef __cplusplus
}
#endif
