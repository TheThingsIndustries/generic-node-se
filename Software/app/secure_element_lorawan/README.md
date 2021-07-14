# secure_element_lorawan

## Description

This application showcases LoRaWAN functionality using a HW secure element (ATECC608A-TNGLORA).
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

## Observation

The device joins via OTAA using the on-board secure element.
