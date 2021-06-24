# freefall\_lorawan

## Description

This application is used to detect free fall events and also serves as a template for acceleration detection handling. Each time a free fall event is detected by the accelerometer, a message containing a counter in the payload is sent. The transmitted value is incremented each time the event occurs. Additionally, if a downlink is received over the correct port, which can be configured by the user, the buzzer will beep periodically to notify locally that the GNSE should be picked up. The buzzer can be turned off by pressing the user button.

## Configuration

### Default
1. Follow [this](https://www.thethingsindustries.com/docs/devices/adding-devices/) guide to add your device to The Things Stack.
2. Change the `APPEUI`, `DEVEUI` and `APPKEY` in [`conf/app_conf.h`](./conf/app_conf.h) to the values specific to your device, which are used on The Things Stack.
3. Uncomment the `REGION` parameters in [`conf/lorawan_conf.h`](./conf/lorawan_conf.h) and set `ACTIVE_REGION` in [`lora_app.h`](./lora_app.h). These should correspond to the location that you use these devices at.

### Free fall detection
1. Change `ACC_FF_THRESHOLD` in [`app/conf.h`](./conf/app_conf.h) to a value you prefer. This will determine the threshold where all axis can be from 0g (which would indicate that it is free falling) to be considered a free fall event.
2. Change `ACC_FF_DURATION` in [`app/conf.h`](./conf/app_conf.h) to a value you prefer. This will determine how many times the axis have to meet the threshold values. Note that `ACC_FF_ODR` in the same file also affects this.
3. `ACC_FF_SCALE` in [`app/conf.h`](./conf/app_conf.h) could also be changed for situations where more than 2g has to be detected. [`LIS2DH12.h`](../../lib/LIS2DH12/LIS2DH12.h) in the [`LIS2DH12`](../../lib/LIS2DH12/) library lists all the options for this in the full scale section.
4. `ACC_FF_ODR` in [`app/conf.h`](./conf/app_conf.h) could be lowered to save power, or increased to improve performance. [`LIS2DH12.h`](../../lib/LIS2DH12/LIS2DH12.h) in the [`LIS2DH12`](../../lib/LIS2DH12/) library lists all the options for this in the `ODR` section.

### Low power

1. Make sure that `LOW_POWER_DISABLE` is kept at `0` in [`conf/app_conf.h`](./conf/app_conf.h).
2. Set `GNSE_ADVANCED_TRACER_ENALBE` to `0` in [`conf/app_conf.h`](./conf/app_conf.h). This will make the device consume less power, but also disables UART.
3. Set `ACC_FF_ODR` in [`app/conf.h`](./conf/app_conf.h) to a lower value if the decreased performance of the free fall detection does not matter for your setup. [`LIS2DH12.h`](../../lib/LIS2DH12/LIS2DH12.h) in the [`LIS2DH12`](../../lib/LIS2DH12/) library lists all the options for this.

Additionally, try to remember that:

- Setting the debugger will consume more power. The debugger is set in the macro `DEBUGGER_ON` in [`conf/app_conf.h`](./conf/app_conf.h).
- Keeping peripherals (such as the sensors) off by controlling the load switches will significantly reduce the power consumption. But, of course, you can not turn the load switch connected to the accelerometer off without disabling the free fall detection as well.

### LoRa extra's

Some extra setup configurations can be done to change the behaviour of the LoRaWAN features. These include:

1. Setting the activation method (OTAA or ABP) in `LORAWAN_DEFAULT_ACTIVATION_TYPE` in [`lora_app.h`](./lora_app.h). OTAA [is recommended](https://www.thethingsindustries.com/docs/devices/abp-vs-otaa/).
2. The data rate can be set in [`lora_app.h`](./lora_app.h). The default configuration uses the ADR. If you want to set your preferred data rate, set `LORAWAN_ADR_STATE` to `LORAMAC_HANDLER_ADR_OFF` and set `LORAWAN_DEFAULT_DATA_RATE` to your preference. A list of the options per region are shown in [`Region.h`](../../lib/STM32WLxx_LoRaWAN/LoRaWAN/Mac/region/Region.h) in the [`STM32WLxx_LoRaWAN`](../../lib/STM32WLxx_LoRaWAN) library.
3. `ACC_FF_LORA_PORT` can be changed in [`conf/app_conf.h`](./conf/app_conf.h), which is used to configure the transmission port. The LoRaWAN keys mentioned in the default section can be altered here as well.
4. Also in [`conf/app_conf.h`](./conf/app_conf.h), the downlink port can be set by altering `ACC_FF_DOWNLINK_PORT`.

### Debugger

For debugging, the firmware has to support it first. The debugger is set in the macro `DEBUGGER_ON` in [`conf/app_conf.h`](./conf/app_conf.h).

## Setup

1. See if your device can detect free fall events. The default values should be enough to trigger on minor drops (try to not break the device) or even sudden movements with your arm whilst holding the GNSE. A message containing the counter should be transmitted, as was mentioned in [`Description`](#description). If none can be seen, try to debug the device as described in the [`Observation`](#observation) section.
2. Test you setup for free fall events and change it to a value you prefer (with the steps in [`Configuration`](#configuration)). Be aware that `ACC_FF_THRESHOLD` would have to be increased to have the accelerometer determine a free fall event has occurred at a lower limit whilst `ACC_FF_DURATION` would have to be decreased for a similar effect.

## Observation

On The Things Stack, the payload should be a single byte starting at one. When a downlink is received on the port set in `ACC_FF_DOWNLINK_PORT` in [`app/conf.h`](./conf/app_conf.h), then the buzzer should start making noises.

The UART on the TX/RX lines can also be used, which are configured to 115200/8-N-1 by default. If you are having troubles with the device, check if the UART output matches these things:

1. The accelerometer is initialized to detect free fall events, which is notified with this message:

```
Accelerometer initialized
```

2. The correct keys are used, i.e. they match the ones printed at the start of the application.
3. The device is connecting properly, as it will indicate if connections have failed or not. It should display something like this:

```
###### = JOINED = OTAA =====================
```

4. The device is transmitting anything, since each event is also logged via the UART lines. It should display something similar to:

```
###### ========== MCPS-Confirm =============
###### U/L FRAME:0005 | PORT:2 | DR:5 | PWR:0 | MSG TYPE:UNCONFIRMED
60s069:TX on freq 868500000 Hz at DR 5
60s080:SEND REQUEST
```

5. Downlinks are detected only if used. These only occur after being explicitly set in The Things Stack Console or another application, and will only be transmitted after the transmission of an uplink by the device. When any data are sent to the port set in `ACC_FF_DOWNLINK_PORT` in [`app/conf.h`](./conf/app_conf.h) then the buzzer can also be heard. Additionally, after receiving the downlink, the GNSE should display something similar to:

```
 ###### ========== MCPS-Indication ==========

 ###### D/L FRAME:0002 | SLOT:1 | PORT:1 | DR:5 | RSSI:-44 | SNR:15

 Received 3 bytes on undefined port: 2
```
