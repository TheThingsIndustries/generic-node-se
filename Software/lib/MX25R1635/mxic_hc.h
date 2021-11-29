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
 * @file mxic_hc.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef MXIC_HC_H_
#define MXIC_HC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mx_define.h"
/*
 * define TransFlag
 */
#define XFER_START 1
#define XFER_END 2

typedef struct
{
    u32 BaseAddress;
    u32 IsReady; /**< Device is initialized and ready */
    u8 *InstrBufPtr;
    u8 LenCmd;
    u8 LenDummy;
    u8 LenAddr;
    u32 CurMode;
    u8 CurAddrMode;
    u8 IsRd;
    u32 CurrFreq;
    u8 WrCmd;
    u8 RdCmd;
    u8 *SendBufferPtr;  /**< Buffer to send (state) */
    u8 *RecvBufferPtr;  /**< Buffer to receive (state) */
    int RequestedBytes; /**< Number of bytes to transfer (state) */
    int RemainingBytes; /**< Number of bytes left to transfer(state) */
    u32 IsBusy;         /**< A transfer is in progress (state) */
    u8 TransFlag;
    u8 FlashProtocol;
    u8 PreambleEn;
    u8 DataPass;
    u8 SopiDqs;
    u8 HardwareMode;
} MxSpi;

enum FlashProtocol
{
    PROT_1_1_1,
    PROT_1_1D_1D,
    PROT_1_1_2,
    PROT_1_1D_2D,
    PROT_1_2_2,
    PROT_1_2D_2D,
    PROT_1_1_4,
    PROT_1_1D_4D,
    PROT_1_4_4,
    PROT_1_4D_4D,
    PROT_4_4_4,
    PROT_4_4D_4D,
    PROT_8_8_8,
    PROT_8D_8D_8D,
};

enum CurrentMode
{
    MODE_SPI = 0x00000001,
    MODE_SOPI = 0x00000002,
    MODE_DOPI = 0x00000004,
    MODE_OPI = (MODE_SOPI | MODE_DOPI),
    MODE_QPI = 0x00000008,
    MODE_FS_READ = 0x00000010,
    MODE_DUAL_READ = 0x00000020,    //PROT_1_1_2
    MODE_DUAL_IO_READ = 0x00000040, //PROT_1_2_2
    MODE_QUAD_READ = 0x00000080,    //PROT_1_1_4
    MODE_QUAD_IO_READ = 0x00000100, //PROT_1_4_4
    MODE_DUAL_WRITE = 0x00000200,
    MODE_DUAL_IO_WRITE = 0x00000400,
    MODE_QUAD_WRITE = 0x00000800,
    MODE_QUAD_IO_WRITE = 0x00001000,
    MODE_FSDT_READ = 0x00002000,
    MODE_DUAL_IO_DT_READ = 0x00004000,
    MODE_QUAD_IO_DT_READ = 0x00008000,
};

enum HardwareMode
{
    IOMode,
    LnrMode,
    LnrDmaMode,
    SdmaMode,
};

int MxHardwareInit(MxSpi *Spi);
int MxPolledTransfer(MxSpi *Spi, u8 *WrBuf, u8 *RdBuf, u32 ByteCount);

#ifdef __cplusplus
}
#endif

#endif /* MXIC_HC_H_ */
