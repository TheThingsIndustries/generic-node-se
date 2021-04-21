# Lib
This folder contains the libraries developed to support the application layer.

[GNSE_BSP](./GNSE_BSP) contains the board support package APIs used to control the device on-board components.

[SHTC3](./SHTC3) contains the Temperature and Humidity sensor [SHTC3](https://www.sensirion.com/en/environmental-sensors/humidity-sensors/digital-humidity-sensor-shtc3-our-new-standard-for-consumer-electronics/) driver and support functions.

[MX25R1635](./MX25R1635) contains the external 16MB external SPI flash[MX25R1635](https://www.macronix.com/Lists/Datasheet/Attachments/7595/MX25R1635F,%20Wide%20Range,%2016Mb,%20v1.6.pdf) driver and support functions.

[LIS2DH12](./LIS2DH12) contains the Accelerometer[LIS2DH12](https://www.st.com/en/mems-and-sensors/lis2dh12.html) driver and support functions.

[BUZZER](./BUZZER) contains the Piezo Buzzer driver and support functions.

[STM32WLxx_LoRaWAN](./STM32WLxx_LoRaWAN) contains the Sub GHz physical layer driver (SX1262 transceiver) and the LoRaWAN stack.

[ATECC608A-TNGLORA](./ATECC608A-TNGLORA) contains the [ATECC608A-TNGLORA](https://www.microchip.com/wwwproducts/en/ATECC608A-TNGLORA) secure element HAL and support functions.

[cryptoauthlib](./cryptoauthlib) contains Microchip support library for ATECC608A-TNGLORA.

[MCU_FLASH](./MCU_FLASH) contains HAL APIs for controlling the SOC internal flash memory.

[FreeRTOS-Kernel](./FreeRTOS-Kernel) contains the FreeRTOS kernel.

[FreeRTOS-LoRaWAN](./FreeRTOS-LoRaWAN) contains the FreeRTOS LoRaWAN abstraction layer.

[SPIFFS](./SPIFFS) contains SPI flash file system library that can be used to abstract external SPI flash operation.

[threadx](./threadx) contains threadx (AzureRTOS) kernel.
