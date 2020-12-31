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
The `idf.sh` script is a helper to pre-select the port your ESP32 is connected to as well as the baud rate it is typically operating at.

These values, especially the port might be different for you, in which case you should modify it in the script.

Note: If you are running this on WSL using Windows 10, while having your ESP32 dev board connected through USB, check your Device Manager under "Ports (COM & LPT)" for e.g. *Silicon Labs CP210x USB UART Bridge(COM**X**)*. This will tell you which /dev/ttyS**X** device to connect to.

## Protocol buffers
We use protocol buffers to encode device settings. We will send these from a Winston Core server
to the nodes so they know what components they have connected (e.g. display, temp sensor, etc)
and how to report these out through MQTT paths.

For this we use [nanopb](https://github.com/nanopb/nanopb) which is a very small C implementation
for protocol buffers. This is used to re-generate the files inside `main/proto`.

After checking out the repo and building it, `generator-bin` should exist inside the directory.
Add this to your `PATH` so you can use the tool.

Inside winston-esp, cd into `main/proto` and run `nanopb_generator ./device_settings.proto && mv *.h include` to re-generate changes from the `device_settings.proto` file.

## Version history

### 0.6 (planned)
 - Receive config from master (which sensors, which pins etc, timezone) ⌛
 - Set up a wifi hotspot to configure wifi connection ⌛
 - OTA support (no more plugging it in to update) ⌛

### 0.5 (planned)
 - Add support for deep sleep mode to enable battery operation ⌛
 - Added support for Bosch BME270 temp/humidity sensor ⌛
 - Added support for Adafruit STEMMA Soil Sensor ⌛


### 0.4  (in progress)
 - Overarching goal: Dynamic config, MQTT, internal events - feature complete with 0.3.
 - Revamped internal event system for modules to publish updates to both MQTT as well as a connected screen ✓
 - Node set-up is controlled by a protocol buffer. This enables nodes to be different by changing their embedded configuration instead of re-compiling. ✓
 - Move timezone setting to be dynamic, instead of in code ✓
 - Added `/io/system/info` call for basic system information ✓
 - Added `/io/system/restart` to restart the system ✓
 - Display: Show MAC address ✓
 - Display: Make connection status more compact ✓
 - Display: Show temp sensor data ✓
 - Added support for PIR motion sensors through interrupts ✓
 - Added support for MQTT, receiving and publishing ✓

### 0.3.1 ✓
 - Fix for webserver becoming unavailable: Ask clients to not keep connections alive
 - Added `/io/system/stats` with task stats to help debugging
  - Note that this needs new config options to prevent linker errors.
 - Move `/io/time` to `/io/system/time`
 - Note: `sdkconfig.defaults` used instead of `sdkconfig`. This means a wifi password will not get accidentally submitted

### 0.3 ✓
 - Support for HTU21D temperature sensor
 - Support for HTU21D humidity sensor
 - Support to get local time through REST interface
 - Switch web server to mongoose to increase stability
 - Introduce locking mechanism and apply to I2C
 - NTP time sync support
 - Show and update date and time on display
 - Show free heap memory on display

### v0.2 ✓
 - Repeated Wifi retry mechanism: Will restart until connected
 - First version of a ui controller to show wifi/network info on screen
 - Support for ssd1306 controlled displays
 - Support for built-in temperature sensor (seems quite inaccurate)
 - Support for built-in hall effect sensor
 - Added new call for relays ("3" or "click") with parameter for duration

### v0.1 ✓
 - First fully working version.
 - WIFI setup, REST server setup.
 - Winston-compatible REST API implementation.
 - Support for relays.
 - Support for reed sensors.
