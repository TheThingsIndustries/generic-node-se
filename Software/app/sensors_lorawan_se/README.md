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

## Setup

Follow this [tutorial](https://www.thethingsindustries.com/docs/devices/claim-atecc608a/) to claim your device, and your device will join via OTAA automatically.

The application can be used with the following Javascript payload formatter:

```javascript
function decodeUplink(input) {
  var data = {};
  data.batt_volt = (input.bytes[0]/10);
  data.temperature = ((input.bytes[1] << 8) + input.bytes[2])/10;
  data.humidity = ((input.bytes[3] << 8) + input.bytes[4])/10;

  return {
    data: data,
  };
}
```
Please see [The Things Stack Javascript payload formatter documentation](https://www.thethingsindustries.com/docs/integrations/payload-formatters/javascript/) for more information.

## Observation

The device joins via OTAA using the on-board secure element and transmits the temperature, humidity and battery voltage information every `SENSORS_TX_DUTYCYCLE`.

