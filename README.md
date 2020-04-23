# winston-esp
The Winston Node implemented for ESP.  (See https://github.com/shaeberling/winston)

## Supported modules.

## Checking out
This project uses git submodules. Make sure you check out using the following command:

```
git clone --recurse-submodules <URL>
```

Or alternatively, if you already had the project checked out, make sure to pull the latest revision and then run:

```
git submodule init
git submodule update
```

See https://git-scm.com/book/en/v2/Git-Tools-Submodules

## Prerequisites
You need to have the ESP-IDF toolchain installed.

## Configuring
The idf.sh script is a helper to pre-select the port your ESP32 is connected to as well as the baud rate it is typically operating at.

These values, especially the port might be different for you, in which case you should modify it in the script.

Note: If you are running this on WSL using Windows 10, while having your ESP32 dev board connected through USB, check your Device Manager under "Ports (COM & LPT)" for e.g. *Silicon Labs CP210x USB UART Bridge(COM**X**)*. This will tell you which /dev/ttyS**X** device to connect to.

## Version history

### 0.3 (future plan, not confirmed)
 - Sets up a wifi hotspot to configure wifi connection ⌛
 - OTA support (no more plugging it in to update) ⌛
 - Receive config through REST call (which sensors, which pins etc) ⌛

### v0.2 (active) ⌛
 - Repeated Wifi retry mechanism: Will restart until connected
 - First version of a ui controller to show wifi/network info on screen
 - Support for ssd1306 controlled displays.
 - Support for built-in temperature sensor (seems quite inaccurate)
 - Support for built-in hall effect sensor
 - Added new call for relays ("3" or "click") with parameter for duration

### v0.1 ✓
 - First fully working version.
 - WIFI setup, REST server setup.
 - Winston-compatible REST API implementation.
 - Support for relays.
 - Support for reed sensors.
