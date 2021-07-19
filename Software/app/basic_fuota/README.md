# basic_fuota

## Description

This application provides a basic example of LoRaWAN FUOTA (Firmware Update Over The Air) functionality based on:

- [LoRaWAN Remote Multicast Setup Specification v1.0.0.](https://lora-alliance.org/resource_hub/lorawan-remote-multicast-setup-specification-v1-0-0/)
- [LoRaWAN Fragmented Data Block Transport Specification v1.0.0.](https://lora-alliance.org/resource_hub/lorawan-fragmented-data-block-transport-specification-v1-0-0/)
- [LoRaWAN Application Layer Clock Synchronization Specification v1.0.0.](https://lora-alliance.org/resource_hub/lorawan-application-layer-clock-synchronization-specification-v1-0-0/)

`basic_fuota` provides an example on how to:

- Synchronize the device clock via `AppTimeReq & Ans`
- Setup a multicast group via `McGroupSetupReq & Ans`
- Setup a class C session via `McClassCsessionReq & Ans`
- Setup a fragmentation session via `FragSessionSetupReq & Ans`
- Transfer fragmented data and do CRC32 check to confirm data correctness

## Configuration

Below are key configurations:

### Fragmentation

This application performs the FUOTA interoperability test, and the number of fragmented bytes can be modified from the default values of 995 bytes in [`FragDecoder.h`](./../../lib/STM32WLxx_LoRaWAN/LoRaWAN/LmHandler/Packages/FragDecoder.h)

```c
#define FRAG_MAX_NB                                 21

#define FRAG_MAX_SIZE                               50
```

### App activity

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

## Setup

### Device setup

- Follow [this](https://www.thethingsindustries.com/docs/devices/adding-devices/) guide to add your device to The Things Stack.

### Server setup

- You can use this example to [test FUOTA server](https://github.com/elsalahy/test-fuota-server) as starting point and you can follow the [`README.md`](https://github.com/elsalahy/test-fuota-server/blob/master/README.md) in order to set it up with a minimal effort.

## Observation

This [video](https://drive.google.com/file/d/1RnhQjbfYMriDybziEVhzww6uxYF-QaNV/view?usp=sharing) provides a recorded example of the communication between the device and the application server.
