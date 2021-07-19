# basic_azurertos

## Description

This application provides a basic example of [Microsoft Azure RTOS](https://azure.microsoft.com/en-us/services/rtos/) functionality.

The application creates two threads and a message queue. Thread 1 attempts to pass a message to thread 2 via the queue and then sleeps for `TX_DELAY`. Thread 2 blinks the LED when the sent message is successfully received.

## Configuration

The application behavior can be adjusted by modifying [`conf/app_conf.h`](./conf/app_conf.h).

Key configurations are stated below:

### Azure RTOS

- `MEM_BYTE_POOL_SIZE`,`THREAD_STACK_SIZE`,`QUEUE_SIZE` in bytes defines Azure RTOS memory pool, threads and queue sizes

```c
#define MEM_BYTE_POOL_SIZE 9120

#define THREAD_STACK_SIZE 1024

#define QUEUE_SIZE 100
```

### App activity

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
