# Application
This folder contains various sample applications.

[basic](./basic) contains the simplest application of on-board peripherals activation to ensure responsiveness and basic functionality of the various components.

[basic_lorawan](./basic_lorawan) contains a simple application of LoRaWAN where a class A device joins via OTAA and sends dummy payloads triggered by a time interval or a push button.
The device enters stop mode in between tranmisions to reduce power consumption.

[basic_bootloader](./basic_bootloader) contains a bootloader that allows for application/internal bootloader jumps.

[basic_freertos](./basic_freertos) contains a basic FreeRTOS low power application that creates two threads and passes messages via a queue triggering a blinking LED with each passed message.

[freefall_lorawan](./freefall_lorawan) contains a simple application to join via OTAA and waits to send a message until the device is free-falling

[freertos_lorawan](./basic_freertos) contains a multi-thread FreeRTOS LoRaWAN Class A demo app that joins over OTAA and sends uplink data.
