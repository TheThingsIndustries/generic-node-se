# Applications

This folder contains various sample applications.

## Applications description

[basic](./basic) contains the simplest application of on-board peripherals activation to ensure responsiveness and basic functionality of the various components.

[basic_lorawan](./basic_lorawan) contains a simple application of LoRaWAN where a class A device joins via OTAA and sends dummy payloads triggered by a time interval or a push button.
The device enters stop mode in between transmissions to reduce power consumption.

[basic_bootloader](./basic_bootloader) contains a bootloader that allows for application/internal bootloader jumps.

[basic_freertos](./basic_freertos) contains a basic FreeRTOS low power application that creates two threads that pass a message via a queue triggering a blinking LED with each passed message.

[freefall_lorawan](./freefall_lorawan) contains a simple application to join via OTAA and waits to send a message until the device is free-falling. A downlink on port 1 will cause the buzzer to beep, which can be turned off with a button press.

[freertos_lorawan](./basic_freertos) contains a multi-thread FreeRTOS LoRaWAN Class A demo app that joins over OTAA and sends uplink data.

[secure_element_lorawan](./secure_element_lorawan) contains a LoRaWAN application where a class A device joins via OTAA (LoRaWAN v1.0.2) using a HW secure element (ATECC608A-TNGLORA) and sends dummy payloads triggered by a time interval.
This app doesn't require LoRaWAN keys/EUI configurations. Follow this [tutorial](https://www.thethingsindustries.com/docs/devices/claim-atecc608a/) to claim your device, and your device will join via OTAA automatically.

[basic_azurertos](./basic_azurertos) contains a simple AzureRTOS (ThreadX) application. It includes examples of two threads that pass a message via a queue triggering a blinking LED with each passed message.

## Applications configuration

Each application contains a configuration folder that facilitates minor adjustments to the application behavior.

LoRaWAN credentials must be configured to ensure successful uplinks/ OTAA joins.
LoRaWAN applications using the onboard secure element require minimum configurations as the keys and EUIs are stored internally within the hardware secure element.

To quickly get started, navigate to your application `app_conf.h`, and adjust the parameters.

[basic app configurations](./basic/conf/app_conf.h)

[basic_lorawan app configurations](./basic_lorawan/conf/app_conf.h)

[basic_bootloader app configurations](./basic_bootloader/conf/app_conf.h)

[basic_freertos app configurations](./basic_freertos/conf/app_conf.h)

[freefall_lorawan app configurations](./freefall_lorawan/conf/app_conf.h)

[freertos_lorawan app configurations](./basic_freertos/conf/app_conf.h)

[secure_element_lorawan app configurations](./secure_element_lorawan/conf/app_conf.h)

[basic_azurertos app configurations](./basic_azurertos/conf/app_conf.h)
