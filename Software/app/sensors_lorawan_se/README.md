# sensors_lorawan_se

## Description

This application samples the on-board device sensors information and transmits the sensor data over LoRaWAN and showcases a HW secure element (ATECC608A-TNGLORA).
You can learn more about LoRaWAN and HW secure elements from [here](https://www.thethingsindustries.com/secure-elements/) and [here](https://www.thethingsindustries.com/docs/devices/atecc608a/).

## Configuration

The application behavior can be adjusted by modifying [`conf/app_conf.h`](./conf/app_conf.h).

- `GNSE_ADVANCED_TRACER_ENABLE` enables UART (115200/8-N-1) logging of application activity

```c
#define GNSE_ADVANCED_TRACER_ENABLE 1
```

- `DEBUGGER_ON` enables the use of a debugger in low power mode

```c
#define DEBUGGER_ON       1
```

> **Note:** Please keep in mind that it is best to disable the tracer and debugger functionalities to reduce power consumption.

- `APPEUI`, `DEVEUI` and `APPKEY` allow the device to join the LoRaWAN network via OTAA.

```c
#define APPEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#define DEVEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#define APPKEY                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
```

> **Note:** The default `0x00` is a place holder and you are required to change these values in order to achieve a successful activation. For testing purposes, these values can be random.

- `SENSORS_PAYLOAD_APP_PORT` defines LoRaWAN application port where sensors information can be retrieved by the application server.

```c
#define SENSORS_PAYLOAD_APP_PORT        2
```

- `SENSORS_TX_DUTYCYCLE` in minutes defines the application data transmission interval.

```c
#define SENSORS_TX_DUTYCYCLE                            10
```


## Setup

Follow this [tutorial](https://www.thethingsindustries.com/docs/devices/claim-atecc608a/) to claim your device, and your device will join via OTAA automatically.

The application can be used with the following Javascript payload formatter:

```javascript
function decodeUplink(input) {
  var data = {};
  data.batt_volt = (input.bytes[0]/10);
  data.temperature = (((input.bytes[1] << 8) + input.bytes[2]) - 500)/10;
  data.humidity = ((input.bytes[3] << 8) + input.bytes[4])/10;
  data.button = input.bytes[5];

  return {
    data: data,
  };
}
```
Please see [The Things Stack Javascript payload formatter documentation](https://www.thethingsindustries.com/docs/integrations/payload-formatters/javascript/) for more information.

## Observation

The device joins via OTAA using the on-board secure element and transmits the temperature, humidity and battery voltage information every `SENSORS_TX_DUTYCYCLE`.

## Secure element usage

By default the app is compiled without secure element, using keys from `conf/app_conf.h`.

To compile the app with secure element support, use the `-DWITH_SECURE_ELEMENT=ON` flag
when generating cmake configuration.

To compile without the secure element and with hardcoded keys instead, 
use the `-DWITH_SECURE_ELEMENT=OFF` flag and define the keys in `conf/app_conf.h`

For example:
```bash
cmake ../.. -DWITH_SECURE_ELEMENT=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../../cross.cmake -DTARGET_APP=sensors_lorawan
```