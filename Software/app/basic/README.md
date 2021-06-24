# basic

## Description

This application is mainly used to test the hardware of the GNSE.

## Configuration

Some settings are available in [`conf/app_conf.h`](./conf/app_conf.h) to set the testing time and amount for some peripherals.

## Observation

The UART output should be read through the TX/RX lines. The default UART configuration is set to 115200/8-N-1. The application will go though some tests, which follows this routine:

1. **Battery**: the output should match the voltage of your battery.
2. **LEDs**: the three on-board LEDs of the board should blink.
3. **Hardware Secure Element**: the application keys, which are unique for each device, should be presented.
4. **Temperature and Humidity sensor**: the outputted data should be similar to your environment's temperature and humidity.
5. **Accelerometer**: the data output consists of three axis corresponding to different directions of the device. When lying still on one side (on its bottom, top or any of the PCB edges) facing downwards, then one axis should have around (-)1g which is equal to (-)9.81m/s², whilst the others would be closer to 0g. This is due to the downwards facing axis being pulled by gravity whilst the others are largely unaffected by it. The PCB has silk screen on the backside pointing to the corresponding directions if you're unsure about which direction it is pointing.
6. **Accelerometer temperature**: the temperature of the accelerometer should match the one of the temperature sensor.
7. **Flash IC**: the flash IC will perform some test to erase, write and read the device. The output will indicate if it has failed at some point.
8. **Buzzer**: buzzer tones will play.
9. **Loop**: the application will enter a loop where the on-board LEDs will blink forever, which is not a test but an indication that the program has finished.
