#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "app.h"
#include "memory_map.h"
#include "MCU_FLASH.h"

#define BOOTLOADER_BTN_PORT BUTTON_SW1_GPIO_PORT
#define BOOTLOADER_BTN_PIN BUTTON_SW1_PIN
#define BOOTLOADER_BTN_SAMPLE_DELAY 100U //in ms
#define BOOTLOADER_BTN_MAX_WAIT 50U // Assuming BOOTLOADER_BTN_SAMPLE_DELAY = 100 ms, wait = 5 seconds
#define BOOTLOADER_BTN_SYS_JMP_WAIT 20U // Assuming BOOTLOADER_BTN_SAMPLE_DELAY = 100 ms, wait = 2 seconds

#define BOOTLOADER_LED_FEEDBACK 1

#if (BOOTLOADER_LED_FEEDBACK)
#define BOOTLOADER_LED_DELAY 500U //in ms
#endif

/* Set vector table location before launching application */
#define SET_VECTOR_TABLE 1

/* Clear reset flags */
#define CLEAR_RESET_FLAGS 1

/* Start address of application space in flash */
#define APP_ADDRESS (uint32_t)(&(__APPROM_START__))

/** Address of System Memory (ST Bootloader) */
#define ST_BOOTLOADER_SYSMEM_ADDRESS (uint32_t)0x1FFF0000

#define BOOTLOADER_BTN_PRESSED() \
    ((HAL_GPIO_ReadPin(BOOTLOADER_BTN_PORT, BOOTLOADER_BTN_PIN) == GPIO_PIN_RESET) ? 1 : 0)

typedef enum
{
    BL_OP_SUCCESS = 0,
    BL_OP_NO_APP,
    BL_OP_SIZE_ERROR,
    BL_OP_CHKSUM_ERROR,
    BL_OP_ERASE_ERROR,
    BL_OP_WRITE_ERROR,
    BL_OP_OBP_ERROR,
    BL_OP_UNKNOWN_ERROR
} Bootloader_op_result_t;

typedef enum
{
    BOOTLOADER_STATE_APP_JMP = 0,
    BOOTLOADER_STATE_APP_CHK = 1,
    BOOTLOADER_STATE_SYS_JMP = 2
} Bootloader_state_t;

uint8_t Bootloader_Init(void);
uint8_t Bootloader_DeInit(void);

void Bootloader_SetState(Bootloader_state_t);
Bootloader_state_t Bootloader_GetState(void);

void Bootloader_HandleInput(void);
void Bootloader_Jump(void);

#endif /* BOOTLOADER_H */
