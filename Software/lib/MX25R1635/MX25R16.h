/** Copyright © 2020 The Things Industries B.V.
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
 * @file MX25R16.h
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#ifndef MX25R16_H_
#define MX25R16_H_

#include "nor_ops.h"
#include "mx_test.h"

int MX25R16_Init(MxChip *Mxic);
int MX25R16_SetMode(MxChip *Mxic, u32 SetMode,u32 SetAddrMode);
int MX25R16_Read(MxChip *Mxic, u32 Addr, u32 SectCnt, u8 *Buf);
int MX25R16_Write(MxChip *Mxic, u32 Addr, u32 SectCnt, u8 *Buf);
int MX25R16_Erase(MxChip *Mxic, u32 Addr, u32 SecCnt);
int MX25R16_LockFlash(MxChip *Mxic, u32 Addr, u64 Len);
int MX25R16_UnlockFlash(MxChip *Mxic, u32 Addr, u64 Len);
int MX25R16_IsFlashLocked(MxChip *Mxic, u32 Addr, u64 Len);
int MX25R16_DPBLockFlash(MxChip *Mxic, u32 Addr, u64 Len);
int MX25R16_DPBUnlockFlash(MxChip *Mxic, u32 Addr, u64 Len);
int MX25R16_IsFlashDPBLocked(MxChip *Mxic, u32 Addr, u64 Len);
int MXR2516_EnterOTP(MxChip *Mxic);
int MXR2516_ExitOTP(MxChip *Mxic);
int MXR2516_DeepPowerDown(MxChip *Mxic);
int MXR2516_RealseDeepPowerDown(MxChip *Mxic);
int MXR2516_Suspend(MxChip *Mxic);
int MXR2516_Resume(MxChip *Mxic);

int MXR2516_BufferRead(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MXR2516_BufferWrite(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);

int MxCalibration(MxChip *Mxic);

#endif /* APP_H_ */
