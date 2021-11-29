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
 * @file MX25R16.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */
#include "MX25R16.h"

/*
 * Function:      MX25R16_Init
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for initializing the device and controller.
 */
int MX25R16_Init(MxChip *Mxic)
{
    int Status;

    Mx_printf("\n\tStart initializing the device and controller\r\n");

    memset(Mxic, 0, sizeof(MxChip));

    Status = MxSoftwareInit(Mxic, BASEADDRESS);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxHardwareInit( Mxic->Priv);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxScanMode(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxChipReset(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    MxSpi *Spi = Mxic->Priv;
    Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
    Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
    Mxic->AppGrp._Erase = (Spi->CurAddrMode==SELECT_3B) ? MxBE : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxBE4B : MxBE);

    return MXST_SUCCESS;
}

#ifdef    BLOCK0_BASIC
/*
 * Function:      MX25R16_SetMode
 * Arguments:      Mxic,        pointer to an mxchip structure of nor flash device.
 *                SetMode,     variable in which to store the operation mode to set.
 *                SetAddrMode, variable in which to store the address mode to set.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for setting the operation mode and address mode.
 */
int MX25R16_SetMode(MxChip *Mxic, u32 SetMode,u32 SetAddrMode)
{
    return MxChangeMode(Mxic, SetMode, SetAddrMode);
}

/*
 * Function:      MX25R16_Read
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                Buf:       pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function issues the Read commands to SPI Flash and reads data from the array.
 *                Data size is specified by ByteCount.
 *                This function invokes MxREAD() when used with Mx25R16 external flash
 */
int MX25R16_Read(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    if ((MxIsFlashBusy(Mxic) == MXST_DEVICE_READY) && (Mxic->WriteBuffStart == FALSE))
    {
        return Mxic->AppGrp._Read(Mxic, Addr, ByteCount, Buf);
    }
    else
    {
        return MXST_DEVICE_BUSY;
    }
}

/*
 * Function:      MX25R16_Write
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to program.
 *                ByteCount: number of bytes to program.
 *                Buf:       pointer to a data buffer where the program data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function programs location to the specified data.
 *                This function invokes MxPP() when used with Mx25R16 external flash
 */
int MX25R16_Write(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return Mxic->AppGrp._Write(Mxic, Addr, ByteCount, Buf);
}

/*
 * Function:      MX25R16_Erase
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of blocks to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function erases the data in the specified Block or Sector.
 *                Function issues all required commands and polls for completion.
 *                This function invokes MxBE() and erases blocks of 64k-byte.
 */
int MX25R16_Erase(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return Mxic->AppGrp._Erase(Mxic, Addr, EraseSizeCount);
}

#endif

#ifdef BLOCK1_SPECIAL_FUNCTION
/*
 * Function:      MXR2516_Suspend.
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function suspends Sector-Erase, Block-Erase or Page-Program operations and conduct other operations.
 */
int MXR2516_Suspend(MxChip *Mxic)
{
    return MxPGMERS_SUSPEND(Mxic);
}

/*
 * Function:      MXR2516_Resume
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function resumes Sector-Erase, Block-Erase or Page-Program operations.
 */
int MXR2516_Resume(MxChip *Mxic)
{
    return MxPGMERS_RESUME(Mxic);
}

/*
 * Function:      MXR2516_DeepPowerDown
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for setting the device on the minimizing the power consumption.
 */
int MXR2516_DeepPowerDown(MxChip *Mxic)
{
    return MxDP(Mxic);
}

/*
 * Function:      MXR2516_RealseDeepPowerDown
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for putting the device in the Stand-by Power mode.
 */
int MXR2516_RealseDeepPowerDown(MxChip *Mxic)
{
    return MxRDP(Mxic);
}

#endif


#ifdef BLOCK2_SERCURITY_OTP
/*
 * Function:      MXR2516_EnterOTP
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for entering the secured OTP mode.
 */
int MXR2516_EnterOTP(MxChip *Mxic)
{
    return MxENSO(Mxic);
}

/*
 * Function:      MXR2516_ExitOTP
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for exiting the secured OTP mode.
 */
int MXR2516_ExitOTP(MxChip *Mxic)
{
    return MxEXSO(Mxic);
}

/*
 * Function:      MX25R16_LockFlash
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of locked area.
 *                Len,   number of bytes to lock.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for writing protection a specified block (or sector) of flash memory.
 */
int MX25R16_LockFlash(MxChip *Mxic, u32 Addr, u64 Len)
{
    return Mxic->AppGrp._Lock(Mxic, Addr, Len);
}

/*
 * Function:      MX25R16_UnlockFlash
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of unlocked area.
 *                Len,   number of bytes to unlock.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function will cancel the block (or sector)  write protection state.
 */
int MX25R16_UnlockFlash(MxChip *Mxic, u32 Addr, u64 Len)
{
    return Mxic->AppGrp._Unlock(Mxic, Addr, Len);
}

/*
 * Function:      MX25R16_IsFlashLocked
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of checking area.
 *                Len,   number of bytes to check.
 * Return Value:  MXST_FAILURE.
 *                   FLASH_IS_UNLOCKED.
 *                FLASH_IS_LOCKED.
 * Description:   This function is for checking if the block (or sector) is locked.
 */
int MX25R16_IsFlashLocked(MxChip *Mxic, u32 Addr, u64 Len)
{
    return Mxic->AppGrp._IsLocked(Mxic, Addr, Len);
}

/*
 * Function:      MX25R16_DPBLockFlash
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of locked area in dynamic protection mode.
 *                Len,   number of bytes to lock in dynamic protection mode.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for writing protection a specified block (or sector) of flash memory in dynamic protection mode.
 */
int MX25R16_DPBLockFlash(MxChip *Mxic, u32 Addr, u64 Len)
{
    return MxDpbLock(Mxic, Addr, Len);
}

/*
 * Function:      MX25R16_DPBUnlockFlash
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of unlocked area in dynamic protection mode.
 *                Len,   number of bytes to unlock in dynamic protection mode.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function will cancel the block (or sector)  write protection state in dynamic protection mode.
 */
int MX25R16_DPBUnlockFlash(MxChip *Mxic, u32 Addr, u64 Len)
{
    return MxDpbUnlock(Mxic, Addr, Len);
}

/*
 * Function:      MX25R16_IsFlashDPBLocked
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of checking area in dynamic protection mode.
 *                Len,   number of bytes to check in dynamic protection mode.
 * Return Value:  MXST_FAILURE.
 *                   FLASH_IS_UNLOCKED.
 *                FLASH_IS_LOCKED.
 * Description:   This function is for checking if the block (or sector) is locked in dynamic protection mode.
 */
int MX25R16_IsFlashDPBLocked(MxChip *Mxic, u32 Addr, u64 Len)
{
    return MxDpbIsLocked(Mxic, Addr, Len);
}

#endif
