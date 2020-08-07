/******************************************************************************
*
* @file spi.c
*
*
* Ver   Who   Date        Changes
* 1.00  AZ      02/23/17    First release
* 1.01  RY      02/27/18    Add DMA mode and Linear mode.
******************************************************************************/

#include "spi.h"

#ifndef SPI_XFER_PERF
#define EXTRA_SZ    30
#define RDWR_BUF_SZ 512
 u8 ReadBuffer[EXTRA_SZ + RDWR_BUF_SZ], WriteBuffer[EXTRA_SZ + RDWR_BUF_SZ];
#endif

/*
 * Function:      MxAddr2Cmd
 * Arguments:      Spi,     pointer to an MxSpi structure of transfer.
 *                Addr,    the address to put into the send data buffer.
 *                CmdBuf,  the data will be send to controller.
 * Return Value:  None.
 * Description:   This function put the value of address into the send data buffer. This address is stored after the command code.
 */
static void MxAddr2Cmd(MxSpi *Spi, u32 Addr, u8 *CmdBuf)
{
    int n;

    for (n = Spi->LenCmd; n <= Spi->LenCmd + Spi->LenAddr - 1; n++)
        CmdBuf[n] = Addr >> (Spi->LenAddr * 8 - (n - Spi->LenCmd + 1) * 8);
}
#ifdef SPI_XFER_PERF
int SpiFlashWrite(MxSpi *Spi, u32 Addr, u32 ByteCount, u8 *WrBuf, u8 WrCmd)
{
    int Status;
    u8 InstrBuf[6] = {0};
    int n;

    Spi->LenCmd = (Spi->CurMode & MODE_OPI) ? 2 : 1;

    for (n = 0; n < Spi->LenCmd; n++)
        InstrBuf[n] = (!n) ? WrCmd : ~WrCmd;
    MxAddr2Cmd(Spi, Addr, InstrBuf);

    Spi->IsRd = FALSE;

    Spi->TransFlag = XFER_START;
    Status = MxPolledTransfer(Spi, InstrBuf, NULL, Spi->LenCmd + Spi->LenAddr + Spi->LenDummy);
    if (Status != MXST_SUCCESS)
        return Status;

    Spi->TransFlag = XFER_END;
    return MxPolledTransfer(Spi, WrBuf, NULL, ByteCount);
}

int MxSpiFlashRead(MxSpi *Spi, u32 Addr, u32 ByteCount, u8 *RdBuf, u8 RdCmd)
{
    int Status;
    u8 InstrBuf[30];
    int n;

    memset(InstrBuf, 0xFF, 30);
    Spi->LenCmd = (Spi->CurMode & MODE_OPI) ? 2 : 1;

    for (n = 0; n < Spi->LenCmd; n++)
        InstrBuf[n] = (!n) ? RdCmd : ~RdCmd;
    MxAddr2Cmd(Spi, Addr, InstrBuf);

    Spi->IsRd = TRUE;

    Spi->TransFlag = XFER_START;
    Status = MxPolledTransfer(Spi, InstrBuf, NULL, Spi->LenCmd + Spi->LenAddr + Spi->LenDummy);
    if (Status != MXST_SUCCESS)
        return Status;

    Spi->TransFlag = XFER_END;
    return MxPolledTransfer(Spi, NULL, RdBuf, ByteCount);
}
#else

/*
 * Function:      SpiFlashWrite
 * Arguments:      Spi,       pointer to an MxSpi structure of transfer
 *                   Addr,      address to be written to
 *                   ByteCount, number of byte to write
 *                   WrBuf,     Pointer to a data buffer where the write data will be stored
 *                   WrCmd,     write command code to be written to the flash
 * Return Value:  MXST_SUCCESS
 *                MXST_FAILURE
 * Description:   This function prepares the data to be written and put them into data buffer,
 *                then call MxPolledTransfer function to start a write data transfer.
 */
int MxSpiFlashWrite(MxSpi *Spi, u32 Addr, u32 ByteCount, u8 *WrBuf, u8 WrCmd)
{
    int n;
    u32 LenInst;
    /*
     * Setup the write command with the specified address and data for the flash
     */
#ifdef BLOCK3_SPECIAL_HARDWARE_MODE
    if((Spi->HardwareMode == IOMode) || (Spi->HardwareMode == SdmaMode))
#endif
    {
        Spi->IsRd = FALSE;
        Spi->LenCmd = (Spi->CurMode & MODE_OPI) ? 2 : 1;
        Spi->TransFlag = XFER_START | XFER_END;
        LenInst = Spi->LenCmd + Spi->LenAddr;

        for (n = 0; n < Spi->LenCmd; n++)
            WriteBuffer[n] = (!n) ? WrCmd : ~WrCmd;
        MxAddr2Cmd(Spi, Addr, WriteBuffer);

        memcpy(WriteBuffer + LenInst, WrBuf, ByteCount);

        return MxPolledTransfer(
                Spi,
                WriteBuffer,
                NULL,
                ByteCount + LenInst);
    }
#ifdef BLOCK3_SPECIAL_HARDWARE_MODE
    else
    {
        /*
         * LnrMode or LnrDmaMode
         */
        return MxLnrModeWrite(Spi, WrBuf, Addr, ByteCount, WrCmd);
    }
#endif
}

/*
 * Function:      SpiFlashRead
 * Arguments:      Spi,       pointer to an MxSpi structure of transfer.
 *                   Addr:      address to be read.
 *                   ByteCount, number of byte to read.
 *                   RdBuf:     pointer to a data buffer where the read data will be stored.
 *                   RdCmd:     read command code to be written to the flash.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function calls MxPolledTransfer function to start a read data transfer,
 *                then put the read data into data buffer.
 */
int MxSpiFlashRead(MxSpi *Spi, u32 Addr, u32 ByteCount, u8 *RdBuf, u8 RdCmd)
{
    int Status;
    int n;
    u32 RdSz, LenInst;
    /*
     * Setup the read command with the specified address, data and dummy for the flash
     */

    Spi->IsRd = TRUE;
    Spi->LenCmd = (Spi->CurMode & MODE_OPI) ? 2 : 1;
    Spi->TransFlag = XFER_START | XFER_END;

    /* Set up the number of dummy cycles, it's dependent on address bus.
     * e.g. (S: single data rate, D: double data rate)
     * 1S-1S-1S, 8 dummy cycles, DUMMY_CNT = 1
     * 1S-4S-4S, 8 dummy cycles, DUMMY_CNT = 4
     * 8D-8D-8D, 8 dummy cycles, DUMMY_CNT = 16
     */
    switch (Spi->FlashProtocol)
    {
        case PROT_1_1_1:
            Spi->LenDummy = Spi->LenDummy / 8;
            break;
        case PROT_1_1D_1D:
            Spi->LenDummy = Spi->LenDummy / 4;
            break;
        case PROT_1_1_2:
            Spi->LenDummy = Spi->LenDummy / 4;
            break;
        case PROT_1_2_2:
            Spi->LenDummy = Spi->LenDummy / 4;
            break;
        case PROT_1_2D_2D:
            Spi->LenDummy = Spi->LenDummy / 2;
            break;
        case PROT_1_1_4:
            Spi->LenDummy = Spi->LenDummy / 2;
            break;
        case PROT_1_4_4:
            Spi->LenDummy = Spi->LenDummy / 2;
            break;
        case PROT_1_4D_4D:
            Spi->LenDummy = Spi->LenDummy ;
            break;
        case PROT_4_4_4:
            Spi->LenDummy = Spi->LenDummy / 2;
            break;
        case PROT_8_8_8:
            Spi->LenDummy = Spi->LenDummy ;
            break;
        case PROT_8D_8D_8D:
            Spi->LenDummy = Spi->LenDummy * 2;
            break;
        default:

            break;
    }
    LenInst = Spi->LenCmd + Spi->LenAddr + Spi->LenDummy;

    for (n = 0; n < Spi->LenCmd; n++)
        WriteBuffer[n] = (!n) ? RdCmd : ~RdCmd;

#ifdef BLOCK3_SPECIAL_HARDWARE_MODE
    if((Spi->HardwareMode == IOMode) || (Spi->HardwareMode == SdmaMode))
#endif
    {
        //    for(; ByteCount; RdBuf += RdSz, Addr += RdSz, ByteCount -= RdSz)
        {
            RdSz = ByteCount > RDWR_BUF_SZ ? RDWR_BUF_SZ : ByteCount;
            MxAddr2Cmd(Spi, Addr, WriteBuffer);
            for (n = Spi->LenCmd + Spi->LenAddr; n < LenInst; n++)
                WriteBuffer[n] = 0xFF;
            memset(ReadBuffer, 0, EXTRA_SZ + RDWR_BUF_SZ);
            Status = MxPolledTransfer(
                    Spi,
                    WriteBuffer,
                    ReadBuffer,
                    RdSz + LenInst);
            if (Status != MXST_SUCCESS)
                return Status;

            if(Spi->HardwareMode == IOMode)
                memcpy(RdBuf, ReadBuffer + LenInst, RdSz);
            else
                memcpy(RdBuf, ReadBuffer, RdSz);
        }
    }
#ifdef BLOCK3_SPECIAL_HARDWARE_MODE
    else
    {
        /*
         * LnrMode or LnrDmaMode
         */
        Status = MxLnrModeRead(Spi, RdBuf, Addr, ByteCount, RdCmd);
        if (Status != MXST_SUCCESS)
            return Status;
    }
#endif
    return MXST_SUCCESS;
}


#endif
