# Generic Node Sensor Edition (GNSE)

## About

LoRaWAN promised you roaming, secure and low-cost sensors that last on a battery for years. _Generic Node_ platform delivers this promise with additional strong end to end encryption and firmware over the air update in a device with multiple sensors that can be configured and programmed to support many use cases.

This repository contains all the necessary hardware and software files for the GNSE LoRaWAN node. The core of the system is based off the STM32WL55xx dual-core LoRaWAN SOC.


![Generic Node Main PCB (front)](Docs/PinoutDiagram/GNSE_v1.1_pinout.png)


## Specification
- [STM32WL55CCU6](https://www.st.com/en/microcontrollers-microprocessors/stm32wl-series.html) dual-core Arm Cortex®-M4/M0+ @48 MHz
- 256 Kbytes of Flash & 64 Kbytes of SRAM
- SX126x sub-GHz radio 
- Power output up to +22.00 dBm 
- LoRa, (G)FSK, (G)MSK, BPSK modulations
- 868 MHz & 915 MHz dual-band operation
- 2xAA battery powered node with wide input voltage range: 2.2V - 5.5V
- [ATECC608A-TNGLORA](https://www.microchip.com/wwwproducts/en/ATECC608A-TNGLORA) pre-provisioned LoRaWAN secure element
- [LIS2DH12](https://www.st.com/en/mems-and-sensors/lis2dh12.html) - ULP 3-axis accelerometer 
- [SHTC3](https://www.sensirion.com/en/environmental-sensors/humidity-sensors/digital-humidity-sensor-shtc3-our-new-standard-for-consumer-electronics/) humidity and temperature sensor
- [MX25R1635](https://www.macronix.com/en-us/products/NOR-Flash/Serial-NOR-Flash/Pages/default.aspx) 16Mb SPI NOR Flash for FUOTA and data-logging
- A user button, an RGB LED, a buzzer and a user-friendly expansion and programming options. 


![Generic Node](Docs/Images/generic_node_se_stacked.png)


## Development

### Hardware
All the necessary schematics and the PCB files can be found in [Hardware](./Hardware).

CAD tools used:
- [KiCAD v.5.1.8](https://kicad.org/)
- [FreeCAD 0.18](https://www.freecadweb.org/)

### Software
Software libraries and sample applications can be found in [Software](./Software)

Build tools used:
- [CMAKE](https://cmake.org/download/)
- [arm-none-eabi-gcc](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- [MinGW](https://osdn.net/projects/mingw/releases/) for `Windows` environments

## Releases

## License
