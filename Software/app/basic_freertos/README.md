# basic_freertos

## Description

This application provides a basic example of [Amazon FreeRTOS](https://docs.aws.amazon.com/freertos/index.html) functionality.

The application creates two threads and a message queue. Thread 1 attempts to pass a message to thread 2 via the queue and then sleeps for `TX_DELAY`. Thread 2 blinks the LED when the sent message is successfully received.

## Configuration

Key configurations are stated below:

### FreeRTOS

FreeRTOS configurations can be adjusted by modifying [`conf/FreeRTOSConfig.h`](./conf/FreeRTOSConfig.h).

```c
#define configMINIMAL_STACK_SIZE                 ((uint16_t)128)

#define configTOTAL_HEAP_SIZE                    ((size_t)8120)
```

### App activity

The application behavior can be adjusted by modifying [`conf/app_conf.h`](./conf/app_conf.h).

- `GNSE_TINY_TRACER_ENABLE` enables UART (115200/8-N-1) logging of application activity

```c
#define GNSE_TINY_TRACER_ENABLE 1
```

- `TX_DELAY` in milliseconds defines the message passing rate from thread 1 to thread 2.

```c
#define TX_DELAY                 (5000)
```

## Observation

The device blinks the blue LED every `TX_DELAY` on each successful message passing between thread 1 and thread 2.
