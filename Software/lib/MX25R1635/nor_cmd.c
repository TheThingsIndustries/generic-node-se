/******************************************************************************
*
* @file nor_cmd.c
*
*
* Ver   Who   Date        Changes
* 1.00  AZ      02/23/17    First release
* 1.01  RY      02/27/18    Divide all code into 4 blocks.
******************************************************************************/
#include "nor_cmd.h"

/*
 * Function:      IsFlashBusy
 * Arguments:      Mxic,   pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_FAILURE.
 *                MXST_DEVICE_BUSY.
 *                MXST_DEVICE_READY.
 * Description:   This function is used for checking status register WIP bit.
 *                If  WIP bit = 1: return MXST_DEVICE_BUSY ( Busy )
 *                            = 0: return MXST_DEVICE_READY ( Ready ).
 */
int MxIsFlashBusy(MxChip *Mxic)
{
    int Status;
    u8 Sr;
    Status = MxRDSR(Mxic, &Sr);
    if (Status != MXST_SUCCESS)
        return Status;
    if(Sr & SR_WIP)
        return MXST_DEVICE_BUSY;
    else
        return MXST_DEVICE_READY;
}

/*
 * Function:      MxWaitForFlashReady
 * Arguments:      Mxic,       pointer to an mxchip structure of nor flash device.
 *                ExpectTime, expected time-out value of flash operations.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is used for waiting for flash ready.
 *                   If flash is ready return MXST_SUCCESS. If flash is time-out return MXST_TIMEOUT.
 */
int MxWaitForFlashReady(MxChip *Mxic,u32 ExpectTime)
{
    MxTime tStart, tCur;
    u32 tUsed;
    MxGetTime(tStart);
    while(MxIsFlashBusy(Mxic) == MXST_DEVICE_BUSY)
    {
        MxGetTime(tCur);
        tUsed = (tCur-tStart)*1000000/COUNTS_PER_SECOND;
        if(tUsed > ExpectTime*3)
        {
            Mx_printf("\t@Warning:MXST_TIMEOUT!!!!!!!!!! >>> \r\n");
            return MXST_TIMEOUT;
        }
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxWaitRYBYReady
 * Arguments:      Mxic,       pointer to an mxchip structure of nor flash device.
 *                ExpectTime, expected time-out value of flash operations.
 * Return Value:  MXST_SUCCESS.
 *                MXST_TIMEOUT.
 * Description:   This function is used for waiting for RYBY pin ready.
 *                  If RYBY pin is high return MXST_SUCCESS.
 */
int MxWaitRYBYReady(MxChip *Mxic,u32 ExpectTime)
{
    MxTime tStart, tCur;
    u32 tUsed;
    MxGetTime(tStart);
    //while(SO == 0)
    {
        MxGetTime(tCur);
        tUsed = (tCur-tStart) * 1000000000 / COUNTS_PER_SECOND;
        if(tUsed > ExpectTime)
            return MXST_TIMEOUT;
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxSetDummyLen
 * Arguments:      Mxic,    pointer to an mxchip structure of nor flash device.
 *                Cmd,     the operation BLOCK.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the dummy cycle of read command.
 */
static int MxSetDummyLen(MxChip *Mxic, u8 Cmd)
{
    MxSpi *Spi = Mxic->Priv;
    u8 CmdNum,ModeNum ;
    u8 CmdDmy[10][3] ={
        /*      SPI  QPI   OPI   */
            { 24,   6, 0xFF},    /* MX_CMD_RES */
        //    { 16, 0xFF, 0xFF},   /* MX_CMD_REMS*/
            { 0, 0xFF, 0xFF},    /* MX_CMD_REMS */
            {  8, 0xFF, 0xFF},   /* MX_CMD_FREAD */
            {  4, 0xFF, 0xFF},   /* MX_CMD_W4READ */
            {  8,    6, 0xFF},   /* MX_CMD_CFIRD */
            {  8,    8,   20},   /* MX_CMD_RDSFDP */
            {(Mxic->ChipSpclFlag & RDPASS_ADDR)?8: 0, 0xFF,   20},   /* MX_CMD_RDPASS */
            {  0,    0,    4},   /* MX_CMD_RDID */
            {  0,    0,    0},
            {  6,    0,    0}
    };

    switch (Cmd) {
        case MX_CMD_RES:       CmdNum = 0; break;
        case MX_CMD_REMS:
        case MX_CMD_REMS2:
        case MX_CMD_REMS4:
        case MX_CMD_REMS4D:    CmdNum = 1; break;
        case MX_CMD_FREAD:
        case MX_CMD_RDDMC:
        case MX_CMD_RDBLOCK2:
        case MX_CMD_RDPLOCK:   CmdNum = 2; break;
        case MX_CMD_W4READ:    CmdNum = 3; break;
        case MX_CMD_CFIRD:     CmdNum = 4; break;
        case MX_CMD_RDSFDP:    CmdNum = 5; break;
        case MX_CMD_RDPASS:    CmdNum = 6; break;
        case MX_CMD_RDSR:
        case MX_CMD_RDCR:
        case MX_CMD_RDCR2:
        case MX_CMD_RDFBR:
        case MX_CMD_RDSCUR:
        case MX_CMD_RDLR:
        case MX_CMD_RDID:      CmdNum = 7; break;
        case MX_CMD_FASTREAD:
        case MX_CMD_FASTREAD4B:
        case MX_CMD_4READ_BOTTOM:
        case MX_CMD_2READ:
        case MX_CMD_2READ4B:
        case MX_CMD_DREAD:
        case MX_CMD_DREAD4B:
        case MX_CMD_QREAD:
        case MX_CMD_QREAD4B:
        case MX_CMD_FASTDTRD:
        case MX_CMD_FASTDTRD4B:
        case MX_CMD_2DTRD:
        case MX_CMD_2DTRD4B:
        case MX_CMD_4DTRD:
        case MX_CMD_RDBUF:
        //case MX_CMD_4DTRD4B:
            Spi->LenDummy = (Spi->PreambleEn) ? 0 : Mxic->RdDummy[Spi->FlashProtocol/2].Dummy;
            return MXST_SUCCESS;
        case MX_CMD_8READ: /* the command BLOCK of 4READ4B and 8READ is the same */
        case MX_CMD_8DTRD: /* the command BLOCK of 4DTRD4B and 8DTRD is the same */
            Spi->LenDummy = (Spi->PreambleEn) ? 4 : Mxic->RdDummy[Spi->FlashProtocol/2].Dummy;
            return MXST_SUCCESS;
        case MX_CMD_RDSPB:
        case MX_CMD_RDDPB:
            if(Spi->CurMode | MODE_OPI )
            {
                Spi->LenDummy = Mxic->RdDummy[Spi->FlashProtocol/2].Dummy;
                return MXST_SUCCESS;
            }
            else{
                 CmdNum = 8; break;
            }

        default: CmdNum = 8;  break;
    }
    switch (Spi->CurMode) {
        case MODE_DOPI:
        case MODE_SOPI:
            ModeNum = 2; break;
        case MODE_QPI:
            ModeNum = 1; break;
        default:
            ModeNum = 0; break;
    }
    Spi->LenDummy = CmdDmy[CmdNum][ModeNum];
    return MXST_SUCCESS;
}

/*
 * Function:      MxSetAddrLen
 * Arguments:      Mxic,    pointer to an mxchip structure of nor flash device.
 *                Cmd,     the operation BLOCK.
 * Return Value:  MXST_SUCCESS.
 * Description:   This function is for setting the device address length of command.
 */
static int MxSetAddrLen(MxChip *Mxic, u8 Cmd)
{
    MxSpi *Spi = Mxic->Priv;
    switch (Cmd) {

    case MX_CMD_READ:
    case MX_CMD_FASTREAD:
    case MX_CMD_PP:
    case MX_CMD_4PP:
    case MX_CMD_SE:
    case MX_CMD_BE:
    case MX_CMD_BE32K:
    case MX_CMD_2READ:
    case MX_CMD_DREAD:
    case MX_CMD_4READ_BOTTOM:
    case MX_CMD_4READ_TOP:
    case MX_CMD_QREAD:
    case MX_CMD_FASTDTRD:
    case MX_CMD_2DTRD:
    case MX_CMD_4DTRD:
        if(Mxic->ChipSpclFlag & ADDR_3BYTE_ONLY)
            Spi->LenAddr = 3;
        else if(Mxic->ChipSpclFlag & ADDR_4BYTE_ONLY)
            Spi->LenAddr = 4;
        else
            Spi->LenAddr = (Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_4B) ? ((Spi->CurAddrMode==SELECT_3B) ? 3:4) : 3;
        break;

    /* 4B mode */
    case MX_CMD_READ4B:
    case MX_CMD_FASTREAD4B:
    case MX_CMD_FASTDTRD4B:
    case MX_CMD_2DTRD4B:
    //case MX_CMD_4DTRD4B:   /* the command BLOCK of 4DTRD4B and MX_CMD_8DTRD is the same */
    case MX_CMD_PP4B:
    case MX_CMD_4PP4B:
    case MX_CMD_SE4B:
    case MX_CMD_BE4B:
    case MX_CMD_BE32K4B:
    case MX_CMD_2READ4B:

    case MX_CMD_4READ4B:    /* the command BLOCK of 4READ4B and 8READ is the same */
    case MX_CMD_DREAD4B:
    case MX_CMD_QREAD4B:
    //case MX_CMD_8READ:
    case MX_CMD_8DTRD:
    case MX_CMD_RDCR2:
    case MX_CMD_WRCR2:
    case MX_CMD_WRSPB:
    case MX_CMD_WRDPB:
    case MX_CMD_RDSPB:
    case MX_CMD_RDDPB:
    case MX_CMD_RDBUF:
    case MX_CMD_WRBI:
    case MX_CMD_WRCT:
        Spi->LenAddr = 4;
        break;

    case MX_CMD_RDPASS:
    case MX_CMD_WRPASS:
    case MX_CMD_PASSULK:
        Spi->LenAddr = (Mxic->ChipSpclFlag & RDPASS_ADDR)? 4 : 0;
        break;
    case MX_CMD_RDSFDP:
        Spi->LenAddr = (Spi->CurMode & MODE_OPI) ? 4 : 3;
        break;

    /* OPI: 4B mode, SPI: 0B mode */
    case MX_CMD_RDID:
    case MX_CMD_RDSR:
    case MX_CMD_RDCR:
    case MX_CMD_RDFBR:
    case MX_CMD_RDSCUR:
    case MX_CMD_RDLR:

    case MX_CMD_SBL:
    case MX_CMD_WRSR:
    //case MX_CMD_WRCR:   /* command BLOCK is same as MX_CMD_WRSR */
    case MX_CMD_WRFBR:
    case MX_CMD_WRLR:
        Spi->LenAddr = (Spi->CurMode & MODE_OPI) ? 4 : 0;
        break;
    case MX_CMD_REMS:
    case MX_CMD_REMS2:
    case MX_CMD_REMS4:
    case MX_CMD_REMS4D:
    case MX_CMD_CP:
    case MX_CMD_SBLK:
    case MX_CMD_SBULK:
        Spi->LenAddr = 3;
        break;

    default:
        Spi->LenAddr = 0;
        break;
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxSetAddrDmyMode
 * Arguments:      Mxic,    pointer to an mxchip structure of nor flash device.
 *                Cmd,     the command BLOCK.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for setting the device address length and dummy cycle of command.
 *                It calls MxSetAddrLen and MxSetDummyLen.
 */
static int MxSetAddrDmyMode(MxChip *Mxic, u8 Cmd)
{
    int Status;
    Status = MxSetDummyLen(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;
    Status = MxSetAddrLen(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;
    return MXST_SUCCESS;
}
/*********************�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{************************
**********************�x      Template for R/W/E      �x************************
**********************�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}***********************/

/*
 * Function:      MxReadTemplate
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                Buf:       pointer to a data buffer where the read data will be stored.
 *                Cmd:       read commands (Single / Fast / Dual / Quad / OCTA) to issue.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function issues the Read commands (Single / Fast / Dual / Quad / OCTA) to SPI Flash and reads data from the array.
 *                It is called by different Read commands functions like MxREAD, Mx8READ and etc.
 */
inline static int MxReadTemplate(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf, u8 Cmd)
{
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 TmpFlashProtocol = Spi->FlashProtocol;
//    Mx_printf("  Cmd: %02X\r\n", Cmd);
    if((Spi->CurMode != MODE_SOPI) && (Spi->CurMode != MODE_DOPI))
    {
        switch (Cmd)
        {
            case MX_CMD_FASTDTRD:
            case MX_CMD_FASTDTRD4B:
                Spi->FlashProtocol = PROT_1_1D_1D;
                break;
            case MX_CMD_DREAD:
            case MX_CMD_DREAD4B:
                Spi->FlashProtocol = PROT_1_1_2;
                break;
            case MX_CMD_2READ:
            case MX_CMD_2READ4B:
                Spi->FlashProtocol = PROT_1_2_2;
                break;
            case MX_CMD_2DTRD:
            case MX_CMD_2DTRD4B:
                Spi->FlashProtocol = PROT_1_2D_2D;
                break;
            case MX_CMD_QREAD:
            case MX_CMD_QREAD4B:
                Spi->FlashProtocol = PROT_1_1_4;
                break;
            case MX_CMD_4READ_BOTTOM:
            case MX_CMD_4READ_TOP:
            case MX_CMD_4READ4B:
                Spi->FlashProtocol = (Spi->CurMode & MODE_QPI)?PROT_4_4_4 : PROT_1_4_4;
                break;
            case MX_CMD_4DTRD:
            case MX_CMD_4DTRD4B:
                Spi->FlashProtocol = (Spi->CurMode & MODE_QPI)? PROT_4_4D_4D : PROT_1_4D_4D;
                break;
            default:
                //Spi->FlashProtocol = PROT_1_1_1;
                break;
        }
    }
    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;
    Status = MxSpiFlashRead(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
    Spi->FlashProtocol = TmpFlashProtocol;
    return Status;
}

/*
 * Function:      MxWriteTemplate
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to program.
 *                ByteCount: number of bytes to program.
 *                Buf:       pointer to a data buffer where the program data will be stored.
 *                Cmd:       write commands (PP/PP4B/4PP/4PP4B) to issue.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function programs location to the specified data.
 *                If the data size to program is larger than PAGE_SIZE, this function will execute page programming operation.
 *                If the page boundary is encountered during page programming,
 *                additional bytes are wrapped around to the start of the same page.
 *                It is called by different write commands functions like MxPP, MxPP4B and etc.
 */
inline static int MxWriteTemplate(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf, u8 Cmd)
{
    int Status;
    int n;
    u32 PageOfs, WrSz;
    MxSpi *Spi = Mxic->Priv;
    u8 TmpFlashProtocol = Spi->FlashProtocol;
    u8 TmpHardwareMode = Spi->HardwareMode;
//    Mx_printf("P  Cmd: %02X\r\n", Cmd);
    PageOfs = Addr & (Mxic->PageSz - 1);

    if (!ByteCount)
        return MXST_SUCCESS;

    Spi->HardwareMode = IOMode;
    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    switch (Cmd)
    {
        case MX_CMD_4PP:
        case MX_CMD_4PP4B:
            Spi->FlashProtocol = PROT_1_4_4;
            break;
        case MX_CMD_QPP:
        //case MX_CMD_QPP4B:
            Spi->FlashProtocol = PROT_1_1_4;
            break;
        default:
            //Spi->FlashProtocol = PROT_1_1_1;
            break;
    }

    if ((PageOfs + ByteCount) <= Mxic->PageSz) {

        Status = MxSetAddrDmyMode(Mxic, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;
        Spi->HardwareMode = TmpHardwareMode;
        Status = MxSpiFlashWrite(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;
        Spi->FlashProtocol = TmpFlashProtocol;

        Spi->HardwareMode = IOMode;
        Status = MxWaitForFlashReady(Mxic, Mxic->tPP);
        if (Status != MXST_SUCCESS)
            return Status;
        Spi->HardwareMode = TmpHardwareMode;
    }
    else {

        WrSz = Mxic->PageSz - PageOfs;

        Status = MxSetAddrDmyMode(Mxic, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;
        Spi->HardwareMode = TmpHardwareMode;
        Status = MxSpiFlashWrite(Mxic->Priv, Addr, WrSz, Buf, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;
        Spi->FlashProtocol = TmpFlashProtocol;
        Spi->HardwareMode = IOMode;
        Status = MxWaitForFlashReady(Mxic, Mxic->tPP);
        if (Status != MXST_SUCCESS)
            return Status;

        for (n = WrSz; n < ByteCount; n += WrSz) {
            WrSz = ByteCount - n;
            if (WrSz > Mxic->PageSz)
                WrSz = Mxic->PageSz;

            //Spi->FlashProtocol = TmpFlashProtocol;
            Status = MxWREN(Mxic);
            Status = MxSetAddrDmyMode(Mxic, Cmd);
            if (Status != MXST_SUCCESS)
                return Status;
            switch (Cmd)
            {
                case MX_CMD_4PP:
                case MX_CMD_4PP4B:
                    Spi->FlashProtocol = PROT_1_4_4;
                    break;
                case MX_CMD_QPP:
                //case MX_CMD_QPP4B:
                    Spi->FlashProtocol = PROT_1_1_4;
                    break;

                default:
                    //Spi->FlashProtocol = PROT_1_1_1;
                    break;
            }
            Spi->HardwareMode = TmpHardwareMode;
            Status = MxSpiFlashWrite(Mxic->Priv, Addr + n, WrSz, Buf + n, Cmd);
            if (Status != MXST_SUCCESS)
                return Status;
            Spi->FlashProtocol = TmpFlashProtocol;
            Spi->HardwareMode = IOMode;
            Status = MxWaitForFlashReady(Mxic, Mxic->tPP);
            if (Status != MXST_SUCCESS)
                return Status;
            Spi->HardwareMode = TmpHardwareMode;
        }
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxEraseTemplate
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block or sector to erase.
 *                Cmd:            erase commands (SE/SE4B/BE/BE4B/BE32K/CE) to issue
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function erases the data in the specified Block or Sector.
 *                Function issues all required commands and polls for completion.
 *                It is called by different erase commands functions like MxSE, MxSE4B and etc.
 */
inline static int MxEraseTemplate(MxChip *Mxic, u32 Addr, u32 EraseSizeCount, u8 Cmd)
{
    int Status;
    int n, AddrStart;
    u32 EraseSize;
    u32 ExpectTime;

    if((Cmd == MX_CMD_BE) || (Cmd == MX_CMD_BE4B))
    {
        EraseSize = BLOCK64KB_SZ;
        ExpectTime = Mxic->tBE;
    }
    else if ((Cmd == MX_CMD_BE32K) || (Cmd == MX_CMD_BE32K4B))
    {
        EraseSize = BLOCK32KB_SZ;
        ExpectTime = Mxic->tBE32;
    }
    else if ((Cmd == MX_CMD_SE) || (Cmd == MX_CMD_SE4B))
    {
        EraseSize = SECTOR4KB_SZ;
        ExpectTime = Mxic->tSE;
    }
    else
    {
        EraseSize =  Mxic-> ChipSz;
        ExpectTime = Mxic->tCE;
    }

    AddrStart = Addr / EraseSize;

    for (n = AddrStart; n < AddrStart + EraseSizeCount; n++) {
        Status = MxWREN(Mxic);
        if (Status != MXST_SUCCESS)
            return Status;

        Status = MxSetAddrDmyMode(Mxic, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;

        Status = MxSpiFlashWrite(Mxic->Priv, n*EraseSize , 0, 0, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;

        Status = MxWaitForFlashReady(Mxic, ExpectTime);
        if (Status != MXST_SUCCESS)
            return Status;
    }
    return MXST_SUCCESS;
}

/*
 * Block 0 : basic command
 */
#ifdef BLOCK0_BASIC
/**********************�x        1.ID commands      �x      *************************/

/*
 * Function:      MxRDID
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                ByteCount: number of ID value to read.
 *                Buf:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the manufacturer ID of 1-byte and followed by device ID of 2-byte.
 */
int MxRDID(MxChip *Mxic, u32 ByteCount, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDID;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 i;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount *= 2;

    Status = MxSpiFlashRead(Spi, 0, ByteCount, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    if (Spi->CurMode == MODE_DOPI)
    {
        for(i=0; i<ByteCount/2; i++)
            Buf[i+1] = Buf[(i+1)*2];
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxRES
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Buf:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the device electric identification of 1-byte.
 */
int MxRES(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RES;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, 1, Buf, Cmd);
}

/*
 * Function:      MxREMS
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      the address determines the sequence of ID.
 *                Buf:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the device manufacturer ID and electric ID of 1-byte.
 */
int MxREMS(MxChip *Mxic,u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_REMS;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, 2, Buf, Cmd);
}

/*
 * Function:      MxREMS2
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      the address determines the sequence of ID.
 *                Buf:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the device manufacturer ID and electric ID of 1-byte in 2 IO mode.
 */
int MxREMS2(MxChip *Mxic,u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_REMS2;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, 2, Buf, Cmd);
}

/*
 * Function:      MxREMS4
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      the address determines the sequence of ID.
 *                Buf:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the device manufacturer ID and electric ID of 1-byte in 4 IO mode.
 */
int MxREMS4(MxChip *Mxic,u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_REMS;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, 2, Buf, Cmd);
}

/*
 * Function:      MxREMS4D
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      the address determines the sequence of ID.
 *                Buf:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the device manufacturer ID and electric ID of 1-byte in 4 IO DT mode.
 */
int MxREMS4D(MxChip *Mxic,u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_REMS;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, 2, Buf, Cmd);
}

/*
 * Function:      MxQPIID
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                ByteCount: number of ID to read.
 *                Buf:       data buffer to store the Status  Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the manufacturer ID of 1-byte and followed by device ID of 2-byte in QPI interface.
 */
int MxQPIID(MxChip *Mxic, u32 ByteCount, u8 *Buf)
{
    u8 Cmd = MX_CMD_QPIID;
    int Status;
    MxSpi *Spi = Mxic->Priv;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Spi, 0, ByteCount, Buf, Cmd);
}


/**********************�x        2.Read commands      �x    *************************/

/*
 * 3/4 byte address command - Read mode
 */
/*
 * Function:      MxREAD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates single IO read command and call MxReadTemplate function.
 */
int MxREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_READ);
}

/*
 * Function:      MxFASTREAD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates single IO fast read command and call MxReadTemplate function.
 */
int MxFASTREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_FASTREAD);
}

/*
 * Function:      MxFASTDTRD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates single IO fast DT read command and call MxReadTemplate function.
 */
int MxFASTDTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_FASTDTRD);
}

/*
 * Function:      Mx2READ
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates two IO read command and call MxReadTemplate function.
 */
int Mx2READ(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_2READ);
}

/*
 * Function:      Mx2DTRD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates two IO DT read command and call MxReadTemplate function.
 */
int Mx2DTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_2DTRD);
}

/*
 * Function:      MxDREAD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates two IO read command and call MxReadTemplate function.
 */
int MxDREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_DREAD);
}

/*
 * Function:      Mx4READ
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates four IO read command and call MxReadTemplate function.
 */
int Mx4READ(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_4READ_BOTTOM);
}

/*
 * Function:      Mx4DTRD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates four IO DT read command and call MxReadTemplate function.
 */
int Mx4DTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_4DTRD);
}

/*
 * Function:      MxQREAD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates four IO read command and call MxReadTemplate function.
 */
int MxQREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_QREAD);
}
/*
 * 4 byte address command - Read mode
 */

/*
 * Function:      MxREAD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates single IO read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int MxREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_READ4B);
}

/*
 * Function:      MxFASTREAD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates single IO fast read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int MxFASTREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_FASTREAD4B);
}

/*
 * Function:      MxFASTDTRD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates single IO fast DT read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int MxFASTDTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_FASTDTRD4B);
}

/*
 * Function:      Mx2READ4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates two IO read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int Mx2READ4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_2READ4B);
}

/*
 * Function:      Mx2DTRD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates two IO DT read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int Mx2DTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_2DTRD4B);
}

/*
 * Function:      MxDREAD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates two IO read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int MxDREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_DREAD4B);
}

/*
 * Function:      Mx4READ4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates four IO read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int Mx4READ4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_4READ4B);
}

/*
 * Function:      Mx4DTRD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates four IO DT read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int Mx4DTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_4DTRD4B);
}

/*
 * Function:      MxQREAD4B
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates four IO read command and call MxReadTemplate function.
 *                The byte number of address should be 4 byte.
 */
int MxQREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_QREAD4B);
}

/*
 * Function:      Mx8READ
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates 8 IO read command and call MxReadTemplate function.
 */
int Mx8READ(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_8READ);
}

/*
 * Function:      Mx8DTRD
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function operates 8 IO DT read command and call MxReadTemplate function.
 */
int Mx8DTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_8DTRD);
}

/*
 * Function:      MxRDBUF
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading write buffer.
 */
int MxRDBUF(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_RDBUF);
}


/*
 * Function:      MxRDSFDP
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 *                Addr:      device address to read.
 *                ByteCount: number of bytes to read.
 *                RdBuf:     pointer to a data buffer where the read data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function can retrieve the operating characteristics, structure and vendor-specified information
 *                such as identifying information, memory size, operating voltages and timing information of device.
 */
int MxRDSFDP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxReadTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_RDSFDP);
}

/**********************�x        3.Program commands      �x *************************/

/*
 * Function:      MxWREN
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for setting Write Enable Latch (WEL) bit.
 */
int MxWREN(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_WREN;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxWRDI
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is to reset Write Enable Latch (WEL) bit.
 */
int MxWRDI(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_WRDI;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * 3/4 byte address command - Write mode
 */

/*
 * Function:      MxPP
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes PP command to programs location to the specified data.
 *                It will call MxWriteTemplate. The byte number of address should be 3 byte.
 */
int MxPP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_PP);
}

/*
 * Function:      Mx4PP
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes 4PP command to programs location to the specified data.
 *                It will call MxWriteTemplate. The byte number of address should be 3 byte.
 */
int Mx4PP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_4PP);
}

/*
 * Function:      MxQPP
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes QPP command to programs location to the specified data.
 *                It will call MxWriteTemplate. The byte number of address should be 3 byte.
 */
int MxQPP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_QPP);
}
/*
 * 4 byte address command - Write mode
 */

/*
 * Function:      MxPP4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes PP4B command to programs location to the specified data.
 *                It will call MxWriteTemplate. The byte number of address should be 4 byte.
 */
int MxPP4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_PP4B);
}

/*
 * Function:      Mx4PP4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes 4PP4B command to programs location to the specified data.
 *                It will call MxWriteTemplate. The byte number of address should be 4 byte.
 */
int Mx4PP4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_4PP4B);
}

/*
 * Function:      MxCP
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for multiple byte program to flash. The device only accept the last
 *                   2 byte to program at a time. It will not roll over during CP mode
 *                It will call MxWriteTemplate. The byte number of address should be 4 byte.
 */
int MxCP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)      //FIXME
{
    int Status;
    int n;

    MxSpi *Spi = Mxic->Priv;
    Status = MxSetAddrDmyMode(Mxic, MX_CMD_CP);
    if (Status != MXST_SUCCESS)
        return Status;

    for (n = 0; n < ByteCount; n += 2) {
        if(n == 0)
            Spi->LenAddr = 3;
        else
            Spi->LenAddr = 0;
        Status = MxSpiFlashWrite(Spi, Addr, 2, Buf + n, MX_CMD_CP);
        if (Status != MXST_SUCCESS)
            return Status;

        if(1)   /* choose hardware method to detect the completion of CP mode by writing ESRY instruction */
        {

        }
        else
        {
            Status = MxWaitForFlashReady(Mxic, Mxic->tBP);
            if (Status != MXST_SUCCESS)
                return Status;
        }
    }
    /* End CP mode */
    Status = MxWRDI(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tBP);
}

/*
 * Function:      Mx8PP
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes PP4B command to programs location to the specified data.
 *                It will call MxWriteTemplate. This function only can be used in SOPI mode.
 */
int Mx8PP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_PP4B);
}

/*
 * Function:      Mx8DTRPP
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function executes PP4B command to programs location to the specified data.
 *                It will call MxWriteTemplate. This function only can be used in DOPI mode.
 */
int Mx8DTRPP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    return MxWriteTemplate(Mxic, Addr, ByteCount, Buf, MX_CMD_PP4B);
}

/*
 * Function:      MxWRBI
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for initiating a write-to-buffer sequence.
 */
int MxWRBI(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    int Status;
    u8 Cmd = MX_CMD_WRBI;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxWRBI
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to program.
 *                ByteCount:      number of bytes to program.
                  Buf:            Pointer to a data buffer where the write data will be stored.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for writing more data to the page buffer in a write-to-buffer sequence.
 */
int MxWRCT(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf)
{
    int Status;
    u8 Cmd = MX_CMD_WRCT;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxWRCF
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for initiating an automatic program operation to write the page buffer data into the array cell.
 */
int MxWRCF(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_WRCF;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/**********************�x        4.Erase commands      �x   *************************/

/*
 * 3/4 byte address command - Erase mode
 */

/*
 * Function:      MxSE4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of sector to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes SE4B command erases the data in the specified Sector.
 */
int MxSE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_SE);
}

/*
 * Function:      MxBE4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes BE4B command erases the data in the specified Block(64K).
 */
int MxBE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_BE);
}

/*
 * Function:      MxBE32K4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes BE4B command erases the data in the specified Block(32K).
 */
int MxBE32K(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_BE32K);
}

/*
 * Function:      MxCE
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes CE command erases the whole chip.
 */
int MxCE(MxChip *Mxic)
{
    return MxEraseTemplate(Mxic, 0, 1, MX_CMD_CE);
}
/*
 * 4 byte address command - Erase mode
 */

/*
 * Function:      MxSE4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of sector to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes SE4B command erases the data in the specified Sector.
 *                The byte number of address should be 4 byte.
 */
int MxSE4B(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_SE4B);
}

/*
 * Function:      MxBE4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes BE4B command erases the data in the specified Block(64K).
 *                The byte number of address should be 4 byte.
 */
int MxBE4B(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_BE4B);
}

/*
 * Function:      MxBE32K4B
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function calls MxEraseTemplate and executes BE4B command erases the data in the specified Block(32K).
 *                The byte number of address should be 4 byte.
 */
int MxBE32K4B(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_BE32K4B);
}

/*
 * Function:      Mx8DTRBE
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is used in DOPI mode ,it calls MxEraseTemplate and executes.
 *                BE4B command erases the data in the specified Block(64K).
 */
int Mx8DTRBE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_BE4B);
}

/*
 * Function:      Mx8BE
 * Arguments:      Mxic:           pointer to an mxchip structure of nor flash device.
 *                Addr:           device address to erase.
 *                EraseSizeCount: number of block to erase.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is used in SOPI mode ,it calls MxEraseTemplate and executes
 *                BE4B command erases the data in the specified Block(64K).
 */
int Mx8BE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount)
{
    return MxEraseTemplate(Mxic, Addr, EraseSizeCount, MX_CMD_BE4B);
}

/**********************�x        5.Register commands      �x*************************/

/*
 * Function:      MxRDSR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the Status Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of Status Register bits.
 */
int MxRDSR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDSR;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 2;
    else
        ByteCount = 1;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxWRSR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the Status Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of Status Register bits.
 */
int MxWRSR(MxChip *Mxic, u8 *Buf)
{Mx_printf("  MxWRSR: %02X\r\n", 0);
    u8 Cmd = MX_CMD_WRSR;
    int Status;
    u8 ByteCount ;
    MxSpi *Spi = Mxic->Priv;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    if (Spi->CurMode == MODE_DOPI)
    {
        ByteCount = 2;
        Buf[1] = Buf[2];
    }
    else
        ByteCount = (Mxic->ChipSpclFlag & SUPPORT_WRSR_CR) ? ((Spi->CurMode & MODE_OPI) ? 1 : 2) : 1;


    Status = MxSpiFlashWrite(Mxic->Priv, 0, ByteCount, Buf, Cmd);
    if (Status != MXST_SUCCESS)
            return Status;
    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxRDCR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the configuration Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of configuration Register bits.
 */
int MxRDCR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDCR;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 2;
    else
        ByteCount = 1;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxWRCR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the configuration Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of configuration Register bits.
 */
int MxWRCR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_WRCR;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
    {
        ByteCount = 2;
        Buf[1] = Buf[0];
    }
    else
        ByteCount = 1;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 1, ByteCount, Buf, Cmd);
    if (Status != MXST_SUCCESS)
            return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxRDCR2
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Addr:    configuration Register2 address to read.
 *                Buf:     data buffer to store the configuration Register2 value
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of configuration Register2 bits.
 */
int MxRDCR2(MxChip *Mxic, u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDCR2;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 2;
    else
        ByteCount = 1;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxWRCR2
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Addr:    configuration Register2 address to write.
 *                Buf:     data buffer to store the configuration Register2 value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of configuration Register2 bits.
 */
int MxWRCR2(MxChip *Mxic, u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_WRCR2;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
    {
        ByteCount = 2;
        Buf[1] = Buf[0];
    }
    else
        ByteCount = 1;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;
    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Spi, Addr, ByteCount, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;
    return Status;
    //return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxRDSCUR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the Security  Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of Security Register bits.
 */
int MxRDSCUR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDSCUR;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 2;
    else
        ByteCount = 1;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxWRSCUR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of Security Register bits.
 */
int MxWRSCUR(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_WRSCUR;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
    if (Status != MXST_SUCCESS)
            return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxEN4B
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for entering 4-byte mode.
 */
int MxEN4B(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_EN4B;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxEX4B
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for exiting 4-byte mode.
 */
int MxEX4B(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_EX4B;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxRSTEN
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for enabling RST command.
 */
int MxRSTEN(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_RSTEN;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxRST
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is used as a system (software) reset that puts the device in normal operating Ready mode.
 */
int MxRST(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_RST;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxRSTQPI
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function resets the device from QPI mode to 1-bit SPI protocol operation.
 */
int MxRSTQPI(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_RSTQPI;
    int Status;
    MxSpi *Spi = Mxic->Priv;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;
    return MxSpiFlashWrite(Spi, 0, 0, 0, Cmd);
}

/*
 * Function:      MxEQPI
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for enabling QPI mode.
 */
int MxEQPI(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_EQPI;
    int Status;
    MxSpi *Spi = Mxic->Priv;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Spi, 0, 0, 0, Cmd);
}

#endif

/*
 * Block 1 : special function command
 */
#ifdef BLOCK1_SPECIAL_FUNCTION

/*
 * Function:      MxCLSR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for clearing security register.
 */
int MxCLSR(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_CLSR;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxRDEAR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the Extended Address Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of Extended Address bits.
 */
int MxRDEAR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDEAR;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, 1, Buf, Cmd);
}

/*
 * Function:      MxWREAR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the Extended Address Register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of Extended Address bits.
 */
int MxWREAR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_WREAR;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 1, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tWREAW);
}

/*
 * Function:      MxWRFBR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the fast boot Register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of fast boot Register bits.
 */
int MxWRFBR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_WRFBR;
    int Status;
    MxSpi *Spi = Mxic->Priv;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    if(Spi->CurMode == MODE_DOPI)
        Spi->DataPass = TRUE;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 4, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    if(Spi->CurMode == MODE_DOPI)
        Spi->DataPass = FALSE;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxRDFBR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the fast boot register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of fast boot register bits.
 */
int MxRDFBR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDFBR;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;
    u8 i;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 8;
    else
        ByteCount = 4;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashRead(Mxic->Priv, 0, ByteCount, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    if (Spi->CurMode == MODE_DOPI)
    {
        for(i=0; i<ByteCount/2; i++)
            Buf[i+1] = Buf[(i+1)*2];
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxESFBR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for erasing the value of fast boot register.
 */
int MxESFBR(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_ESFBR;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxRDFSR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the Factory Status Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of Factory Status register bits.
 */
int MxRDFSR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDFSR;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, 1, Buf, Cmd);
}

/*
 * Function:      MxDP
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for setting the device on the minimizing the power consumption.
 */
int MxDP(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_DP;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxRDP
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for putting the device in the Stand-by Power mode.
 */
int MxRDP(MxChip *Mxic)
{
    if(Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_RDP)
    {
        u8 Cmd = MX_CMD_RDP;
        int Status;

        Status = MxSetAddrDmyMode(Mxic, Cmd);
        if (Status != MXST_SUCCESS)
            return Status;

        return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
    }
    else
    {
        /* if flash does not support,just set CS pin low ,then set CS pin high to exit DP mode*/

    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxEXSA
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for entering the secured Area mode.
 */
int MxENSA(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_ENSA;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxEXSA
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for exiting the secured Area mode.
 */
int MxEXSA(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_EXSA;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxESRY
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for outputting the ready/busy status to SO during CP mode.
 */
int MxESRY(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_ESRY;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxDSRY
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for resetting ESRY during CP mode.
 */
int MxDSRY(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_DSRY;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxWPSEL
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   When the system accepts and executes WPSEL instruction, the bit 7 in security register (WPSEL) will be set.
 */
int MxWPSEL(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_WPSEL;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);

    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tPP);
}

/*
 * Function:      MxSBL
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for setting the burst length.
 */
int MxSBL(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_SBL;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 1, Buf, Cmd);
}

/*
 * Function:      MxFMEN
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for enhancing Program and Erase performance and increasing factory production throughout.
 */
int MxFMEN(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_FMEN;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}


/*
 * Function:      MxPGMERS_SUSPEND
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function suspends Sector-Erase, Block-Erase or Page-Program operations and conduct other operations.
 */
int MxPGMERS_SUSPEND(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_PGMERS_SUSPEND;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxPGMERS_RESUME
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function resumes Sector-Erase, Block-Erase or Page-Program operations.
 */
int MxPGMERS_RESUME(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_PGMERS_RESUME;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxRDBLOCK
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address.
 *                Buf,   data buffer to store the Register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the status of protection lock of a specified block (or sector).
 *                This instruction is only effective after WPSEL was executed.
 */
int MxRDBLOCK(MxChip *Mxic,u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDBLOCK;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, Addr, 1, Buf, Cmd);
}

/*
 * Function:      MxNOP
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is null operation of flash.
 */
int MxNOP(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_NOP;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

#endif

/*
 * Block 2 : security/OTP command
 */
#ifdef BLOCK2_SERCURITY_OTP
/*
 * Function:      MxWRLR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the lock register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of lock register bits.
 */
int MxWRLR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_WRLR;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 2, Buf, Cmd);
    if (Status != MXST_SUCCESS)
            return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxRDLR
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the lock register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of lock register bits.
 */
int MxRDLR(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDLR;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, 2, Buf, Cmd);
}

/*
 * Function:      MxWRPASS
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the password register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of password register bits.
 */
int MxWRPASS(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_WRPASS;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 8, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tPP);
}

/*
 * Function:      MxRDPASS
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the password register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of password register bits.
 */
int MxRDPASS(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDPASS;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, 8, Buf, Cmd);
}

/*
 * Function:      MxPASSULK
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the password register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for password unlock.
 */
int MxPASSULK(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_PASSULK;
    int Status;

    Mx_printf("\t\t start entering password\r\n", 0);

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 8, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tPP);
}

/*
 * Function:      MxWRSPB
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Addr:    the address of SPB register.
 *                Buf:     data buffer to store the SPB register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of SPB register bits.
 */
int MxWRSPB(MxChip *Mxic, u32 Addr)
{
    u8 Cmd = MX_CMD_WRSPB;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, Addr, 0, 0, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tPP);
}

/*
 * Function:      MxRDSPB
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Addr:    the address of SPB register.
 *                Buf:     data buffer to store the SPB register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of SPB register bits.
 */
int MxRDSPB(MxChip *Mxic, u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDSPB;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 2;
    else
        ByteCount = 1;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxESSPB
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for erasing the value of SPB register.
 */
int MxESSPB(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_ESSPB;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, NULL, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tSE);
}

/*
 * Function:      MxSPBLK
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for is setting SPB lock.
 */
int MxSPBLK(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_SPBLK;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, NULL, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tPP);
}

/*
 * Function:      MxRDSPBLK
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Buf:     data buffer to store the SPB lock register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of SPB lock register bits.
 */
int MxRDSPBLK(MxChip *Mxic, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDSPBLK;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, 0, 1, Buf, Cmd);
}

/*
 * Function:      MxWRDPB
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Addr:    the address of DPB register.
 *                Buf:     data buffer to store the DPB register value to write.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for setting the value of DPB register bits.
 */
int MxWRDPB(MxChip *Mxic, u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_WRDPB;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
    {
        ByteCount = 2;
        Buf[1] = Buf[0];
    }
    else
        ByteCount = 1;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tPP);
}

/*
 * Function:      MxRDDPB
 * Arguments:      Mxic:    pointer to an mxchip structure of nor flash device.
 *                Addr:    the address of DPB register.
 *                Buf:     data buffer to store the DPB register value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for reading the value of DPB register bits.
 */
int MxRDDPB(MxChip *Mxic, u32 Addr, u8 *Buf)
{
    u8 Cmd = MX_CMD_RDDPB;
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 ByteCount;

    if (Spi->CurMode == MODE_DOPI)
        ByteCount = 2;
    else
        ByteCount = 1;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashRead(Mxic->Priv, Addr, ByteCount, Buf, Cmd);
}

/*
 * Function:      MxSBLK
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for writing protection a specified block (or sector) of flash memory.
 *                This instructions are only effective after WPSEL was executed.
 */
int MxSBLK(MxChip *Mxic,u32 Addr)
{
    u8 Cmd = MX_CMD_SBLK;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, Addr, 0, 0, Cmd);

    if (Status != MXST_SUCCESS)
        return Status;
    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxSBULK
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function will cancel the block (or sector)  write protection state.
 *                This instructions are only effective after WPSEL was executed.
 */
int MxSBULK(MxChip *Mxic,u32 Addr)
{
    u8 Cmd = MX_CMD_SBULK;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, Addr, 0, 0, Cmd);

    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxGBLK
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for enabling the lock protection block of the whole chip.
 *                This instruction is only effective after WPSEL was executed.
 */
int MxGBLK(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_GBLK;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxGBULK
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 *                MXST_TIMEOUT.
 * Description:   This function is for disabling the lock protection block of the whole chip.
 *                This instruction is only effective after WPSEL was executed.
 */
int MxGBULK(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_GBULK;
    int Status;

    Status = MxWREN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    Status = MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);

    if (Status != MXST_SUCCESS)
        return Status;

    return MxWaitForFlashReady(Mxic, Mxic->tW);
}

/*
 * Function:      MxENSO
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for entering the secured OTP mode.
 */
int MxENSO(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_ENSO;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

/*
 * Function:      MxEXSO
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for exiting the secured OTP mode.
 */
int MxEXSO(MxChip *Mxic)
{
    u8 Cmd = MX_CMD_EXSO;
    int Status;

    Status = MxSetAddrDmyMode(Mxic, Cmd);
    if (Status != MXST_SUCCESS)
        return Status;

    return MxSpiFlashWrite(Mxic->Priv, 0, 0, 0, Cmd);
}

#endif
