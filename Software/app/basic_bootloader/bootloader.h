#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "app.h"
#include "memory_map.h"
#include "MCU_FLASH.h"

/* Set vector table location before launching application */
#define SET_VECTOR_TABLE 1

/* Clear reset flags */
#define CLEAR_RESET_FLAGS 1

/* Start address of application space in flash */
#define APP_ADDRESS (uint32_t)(&(__APPROM_START__))

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

void Bootloader_JumpToApplication(void);


#endif /* BOOTLOADER_H */
