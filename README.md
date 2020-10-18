# Community WiFi repeater

## Overview

An ESP32 firmware allowing to connect to a community WiFi network (FreeWifi, SFR WiFi Public) and to repeat it in a private WiFi network.

## Compatibility

This project was tested and developed on [ESP32-WROOM-32](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) ([this reference on Amazon](https://www.amazon.fr/gp/product/B071P98VTG)).

## Inspirations

The code is based on the [Martin Ger's ESP32 NAT router project](https://github.com/martin-ger/esp32_nat_router) itself inspired by [Console Component](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/console.html#console) and [esp-idf-nat-example](https://github.com/jonask1337/esp-idf-nat-example).
 
## Supported community networks

For the moment, this project only implements access to two French/European community WiFi networks:

- [FreeWifi](https://wifi.free.fr/)
- [SFR Wifi Public](https://hotspot.wifi.sfr.fr/) (compatible with [FON WiFi networks](https://fon.com/))

Feel free to submit a pull request to add support for another network.

## Configurations

This section is TODO

## Build

First, install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-2-get-esp-idf), then

```
get_idf
idf.py -p /dev/ttyUSB0 flash
idf.py monitor
```

## Licence

This section is TODO
