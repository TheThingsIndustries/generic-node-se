# basic\_lorawan

## Description

`basic_lorawan` serves as a basic template for LoRaWAN applications.

## Configuration

### Default

1. Follow [this](https://www.thethingsindustries.com/docs/devices/adding-devices/) guide to add your device to The Things Stack.
2. Change the `APPEUI`, `DEVEUI` and `APPKEY` in [`conf/app_conf.h`](./conf/app_conf.h) to the values specific to your device, which are used on The Things Stack.
3. Uncomment the `REGION` parameters in [`conf/lorawan_conf.h`](./conf/lorawan_conf.h) and set `ACTIVE_REGION` in [`lora_app.h`](./lora_app.h). These should correspond to the location that you use these devices at.

### Low power

1. Make sure that `LOW_POWER_DISABLE` is kept at `0` in [`conf/app_conf.h`](./conf/app_conf.h).
2. Set `GNSE_ADVANCED_TRACER_ENALBE` to `0` in [`conf/app_conf.h`](./conf/app_conf.h). This will make the device consume less power, but also disables UART.

Additionally, try to remember that:

- Setting the debugger will consume more power. The debugger is set in the macro `DEBUGGER_ON` in [`conf/app_conf.h`](./conf/app_conf.h).
- Keeping peripherals (such as the sensors) off by controlling the load switches will significantly reduce the power consumption.
- The less transmissions over time, the lower the power consumption will be. This is set by `APP_TX_DUTYCYCLE` in [`lora_app.h`](./lora_app.h).

### LoRa extra's

Some extra setup configurations can be done to change the behaviour of the LoRaWAN features. These include:

1. Setting the activation method (OTAA or ABP) in `LORAWAN_DEFAULT_ACTIVATION_TYPE` in [`lora_app.h`](./lora_app.h). OTAA [is recommended](https://www.thethingsindustries.com/docs/devices/abp-vs-otaa/).
2. The data rate can be set in [`lora_app.h`](./lora_app.h). The default configuration uses the ADR. Should you want to set your preferred data rate, set `LORAWAN_ADR_STATE` to `LORAMAC_HANDLER_ADR_OFF` and set `LORAWAN_DEFAULT_DATA_RATE` to your preference. A list of the options per region are shown in [`Region.h`](../../lib/STM32WLxx_LoRaWAN/LoRaWAN/Mac/region/Region.h) in the [`STM32WLxx_LoRaWAN`](../../lib/STM32WLxx_LoRaWAN) library.
3. `ACC_FF_LORA_PORT` can be changed in [`conf/app_conf.h`](./conf/app_conf.h), which is used to configure the transmission port. The LoRaWAN keys mentioned in the default section can be altered here as well.

### Debugger

For debugging, the firmware has to support it first. The debugger is set in the macro `DEBUGGER_ON` in [`conf/app_conf.h`](./conf/app_conf.h).

## Observation

The standard payload is "**AA BB CC**", which will repeat periodically, with intervals set in `APP_TX_DUTYCYCLE` in [`lora_app.h`](./lora_app.h). The UART output can be read through the TX/RX lines, which can be used for debugging. The default UART configuration is set to 115200/8-N-1. If nothing can be seen on The Things Stack Console, then you could check from this debug output if:

1. The correct keys are used, i.e. they match the ones printed at the start of the application.
2. The device is connecting properly, as it will indicate if connections have failed or not. It should display something like this:

```
###### = JOINED = OTAA =====================
```

3. The device is transmitting anything, since each event is also logged via the UART lines. It should display something similar to:

```
###### ========== MCPS-Confirm =============
###### U/L FRAME:0005 | PORT:2 | DR:5 | PWR:0 | MSG TYPE:UNCONFIRMED
60s069:TX on freq 868500000 Hz at DR 5
60s080:SEND REQUEST
```

4. Downlinks are detected only if used. These only occur after being explicitly set on The Things Stack Console or another application, and will only be transmitted after the transmission of an uplink by the device. The GNSE should, after receiving the downlink, display something similar to:

```
 ###### ========== MCPS-Indication ==========

 ###### D/L FRAME:0002 | SLOT:1 | PORT:1 | DR:5 | RSSI:-44 | SNR:15

 Received 3 bytes on undefined port: 2
```
