
/** Copyright © 2021 The Things Industries B.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file mx_define.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */
#ifndef MX_DEFINE_H
#define MX_DEFINE_H

#define PLATFORM_GNSE 1

#if PLATFORM_GNSE
#include "GNSE_tracer.h"
#include "GNSE_bsp.h"

#define GetChar()
#define Mx_printf LIB_PRINTF
#define MxTime uint32_t
#define MxGetTime(TimeVal) do{ {TimeVal = HAL_GetTick();}} while(0);
#define COUNTS_PER_SECOND 1000U //Because HAL_GetTick() returns ms values and the driver uses us
#else
#error "Unsupported/Undefined platform!"
#endif

#define BASEADDRESS     0x00 //Added for compatability with MxSoftwareInit

typedef unsigned char u8;        /**< unsigned 8-bit */
typedef char int8;                /**< signed 8-bit */
typedef unsigned short u16;        /**< unsigned 16-bit */
typedef short int16;            /**< signed 16-bit */
typedef unsigned long u32;        /**< unsigned 32-bit */
typedef unsigned long long u64; /**< unsigned 32-bit */
typedef long int32;                /**< signed 32-bit */
typedef float Xfloat32;            /**< 32-bit floating point */
typedef double Xfloat64;        /**< 64-bit double precision FP */
typedef unsigned long Xboolean; /**< boolean (XTRUE or XFALSE) */

enum DeviceStatus
{
    MXST_SUCCESS = 0L,
    MXST_FAILURE = 1L,
    MXST_TIMEOUT = 2L,
    MXST_DEVICE_IS_STARTED = 3L,
    MXST_DEVICE_IS_STOPPED = 4L,
    MXST_ID_NOT_MATCH = 5L,
    MXST_DEVICE_BUSY = 6L,    /* device is busy */
    MXST_DEVICE_READY = 7L, /* device is ready */
    MXST_DEVICE_SPBLK_PROTECTED = 8L,
    MXST_DEVICE_SPB_IS_LOCKED = 9L,
    MXST_DEVICE_SPB_ISNOT_LOCKED = 10L,
};

typedef struct
{
    u8 CRValue;
    u8 Dummy;
} RdDummy;

#define TRUE 1U
#define FALSE 0U
//#define NULL        0

#define MX_COMPONENT_IS_READY 0x11111111U    /**< component has been initialized */
#define MX_COMPONENT_IS_STARTED 0x22222222U /**< component has been started */

#define TEST_SZ_PERFORMANCE 256
#define TEST_SZ 32
#define PAGE_SZ 256
#define SECTOR4KB_SZ 0x1000
#define BLOCK32KB_SZ 0x8000
#define BLOCK64KB_SZ 0x10000

#define SPI_NOR_FLASH_MAX_ID_LEN 6
#define PASSWORD_LEN 8
#define PASSWORD_INIT_VALUE 0xFF

#define BLOCK0_BASIC
/*
 * Include command:
 *         Read     command:  READ, 2READ, 4READ, 4READ_TOP, FASTREAD, DREAD,QREAD, FASTDTRD, 2DTRD, 4DTRD,
 *                           FRDTR4B, 2DTRD4B, 4DTRD4B,W4READ, CFIRD, RDDMC, RDSFDP, READ4B, FASTREAD4B,
 *                           2READ4B, 4READ4B, FREAD, DREAD4B, QREAD4B,
 *
 *         Program  command:  WREN, WRDI, PP, 4PP, QPP, CP, PP4B, 4PP4B
 *
 *         Erase    command:  SE, SE4B, BE32K, BE32K4B, BE, BE4B, CE
 *
 *         ID       command:  RDID, RES, REMS, REMS2, REMS4, REMS4D, QPIID
 *
 *         Register and function command:  WRSR, RDSR,WRSCUR, RDSCUR, RDCR, RDCR2, WRCR2, EN4B, EX4B, RSTEN, RST, RSTQIO, EQIO,
 *
 */

#define BLOCK1_SPECIAL_FUNCTION // Not supported with our MX25R1635F flash, but we must define it for OTP commands
/*
 * Include command:
 *     special function command:
 *        CLSR,
 *        RDEAR, WREAR,
 *        RDFBR, WRFBR, ESFBR
 *         DP, RDP,
 *         ENSA, EXSA,
 *         ESRY, DSRY,
 *         SBL,
 *         ENPLM, EXPLM,
 *         HDE,
 *         FMEN,
 *         PGM_ERS_S, PGM_ERS_R,
 *         NOP,
 */

#define BLOCK2_SERCURITY_OTP
/*
 * Include command:
 *     security (lock/unlock) command:
 *        WPSEL,
 *         WRLR, RDLR,
 *        WRPASS, RDPASS, PASSULK,
 *        RDSPB, WRSPB, ESSPB,
 *        SPBLK, RDSPBLK,
 *        WRDPB, RDDPB,
 *        SBLK, SBULK,
 *        GBLK, GBULK,
 *        KEY1, KEY2,
 *        BLOCKP, RDBLOCK, RDBLOCK2,
 *        UNLOCK, WRLB, PLOCK, RDPLOCK, WRLCR, RRLCR, PLLK, RDPLLK,
 *  OTP command:
 *         ENSO, EXSO
 */
// #define BLOCK3_SPECIAL_HARDWARE_MODE
/*
 * Include :
 *     LnrMode,
 *    LnrDmaMode,
 *    SdmaMode,
 */

// #define BLOCK4_TESTBENCH
/*
 * Include file about test:
 *     mx_test.c
 *     mx_test.h
 *     user_test.c
 *     user_test.h
 *     cmd_test.c
 *     cmd_test.h
 *     hardware_test.c
 *     hardware_test.h
 */

#endif
