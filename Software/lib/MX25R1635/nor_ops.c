/******************************************************************************
*
* @file nor_ops.c
*
*
* Ver   Who   Date        Changes
* 1.00  AZ      02/23/17    First release
* 1.01  RY      02/27/18    Add DMA function and divide all code into 4 blocks.
******************************************************************************/

#include "nor_ops.h"
#include "math.h"

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define INFO(_JedecId, _ExtId, _BlockSz, _N_Blocks,                                                 \
             _SupModeFlag,_SpclFlag,                                                                \
             _CR_Dummy_0,_CR_Dummy_1,_CR_Dummy_2,_CR_Dummy_3,_CR_Dummy_4,_CR_Dummy_5,_CR_Dummy_6,    \
             _SPICmdList0,_SPICmdList1,_SPICmdList2,_SPICmdList3,                                    \
             _QPICmdList0,_QPICmdList1,_QPICmdList2,_QPICmdList3,                                    \
             _OPICmdList0,_OPICmdList1,_OPICmdList2,_OPICmdList3,                                    \
             _tW,_tDP,_tBP,_tPP,_tSE,_tBE32,_tBE,_tCE,_tWREAW)                                         \
    .Id = {                                                    \
            ((_JedecId) >> 16) & 0xff,                        \
            ((_JedecId) >>  8) & 0xff,                        \
            (_JedecId) & 0xff,                                \
            ((_ExtId) >> 8) & 0xff,                         \
            (_ExtId) & 0xff,                                 \
          },                                                \
    .IdLen = (!(_JedecId) ? 0 : (3 + ((_ExtId) ? 2 : 0))),     \
    .BlockSz = (_BlockSz),                                    \
    .N_Blocks = (_N_Blocks),                                \
    .PageSz = PAGE_SZ,                                         \
    .SupModeFlag = (_SupModeFlag),                          \
    .SpclFlag = (_SpclFlag),                                \
    .RdDummy = {                                            \
                {((_CR_Dummy_0)>>8) & 0xff,((_CR_Dummy_0)>>0) & 0xff},                              \
                {((_CR_Dummy_1)>>8) & 0xff,((_CR_Dummy_1)>>0) & 0xff},                              \
                {((_CR_Dummy_2)>>8) & 0xff,((_CR_Dummy_2)>>0) & 0xff},                              \
                {((_CR_Dummy_3)>>8) & 0xff,((_CR_Dummy_3)>>0) & 0xff},                              \
                {((_CR_Dummy_4)>>8) & 0xff,((_CR_Dummy_4)>>0) & 0xff},                              \
                {((_CR_Dummy_5)>>8) & 0xff,((_CR_Dummy_5)>>0) & 0xff},                              \
                {((_CR_Dummy_6)>>8) & 0xff,((_CR_Dummy_6)>>0) & 0xff}                               \
               },                                                                                    \
     .SPICmdList = {_SPICmdList0,_SPICmdList1,_SPICmdList2,_SPICmdList3},                              \
     .QPICmdList = {_QPICmdList0,_QPICmdList1,_QPICmdList2,_QPICmdList3},                              \
     .OPICmdList = {_OPICmdList0,_OPICmdList1,_OPICmdList2,_OPICmdList3},                            \
     .tW     =_tW,                                            \
     .tDP    =_tDP,                                            \
     .tBP    =_tBP,                                           \
     .tPP    =_tPP,                                           \
     .tSE    =_tSE,                                         \
     .tBE32  =_tBE32,                                        \
     .tBE    =_tBE,                                            \
     .tCE    =_tCE,                                            \
     .tWREAW =_tWREAW,

typedef struct {
    char *name;
    u8 Id[SPI_NOR_FLASH_MAX_ID_LEN];
    u8 IdLen;
    u32 BlockSz;
    u16 N_Blocks;
    u16 PageSz;
    u16 AddrLen;
    u32 SupModeFlag;
    u32 SpclFlag;
    RdDummy RdDummy[7] ;
    u32 SPICmdList[4];
    u32 QPICmdList[4];
    u32 OPICmdList[4];
    u32 tW;
    u32 tDP;
    u32 tBP;
    u32 tPP;
    u32 tSE;
    u32 tBE32;
    u32 tBE;
    u32 tCE;
    u32 tWREAW;
}MxFlashInfo;

static MxFlashInfo SpiFlashParamsTable[] = {
        {"mx25r1635f", INFO(
                               0xC22815, 0x15,                           /* ID */
                               64 * 1024, 32,                           /*block size / block count */
                               MODE_SPI|MODE_FS_READ|MODE_DUAL_READ|MODE_DUAL_IO_READ|MODE_QUAD_READ|MODE_QUAD_IO_READ, /*support mode*/
                               SUPPORT_WRSR_CR,                          /*special flag*/

                               /** bit15-8 (DC Value); bit7-0 (Dummy Cycle) ;     FF means this read command is not supported **/
                               0x0008,        /**Fast Read:             1S-1S-1S   //  Fast DTR Read(FASTDTRD): 1S-1D-1D **/
                               0x0008,        /**Dual Read     (DREAD): 1S-1S-2S                                         **/
                               0x0108,          /**Dual IO Read  (2READ): 1S-2S-2S   //  Dual IO Read  (2DTRD):   1S-2D-2D **/
                               0x0008,           /**Quad Read     (QREAD): 1S-1S-4S                                          **/
                               0x010A,          /**Quad IO Read  (4READ): 1S-4S-4S   //  Dual IO Read  (2DTRD):   1S-4D-4D **/
                               0xFFFF,          /**QPI mode Read:         4S-4S-4S                                         **/
                               0xFFFF,          /**SOPI mode Read(8READ): 8S-8S-8S   //  DOPI mode Read(8DTRD):   8D-8D-8D **/

                               /*SPI mode command list*/
                               MX_RDID|MX_RES|MX_REMS|MX_WRSR|MX_WRSCUR|MX_RDSCUR|MX_RDCR,
                               MX_READ|MX_FASTREAD|MX_2READ|MX_DREAD|MX_4READ|MX_QREAD|MX_RDSFDP,
                               MX_PP|MX_4PP|MX_SE|MX_BE|MX_BE32K|MX_CE,
                               MX_DP|MX_SBL|MX_SUS_RES,
                               /*QPI mode command list*/
                               0,0,0,0,
                               /*OPI mode command list*/
                               0,0,0,0,
                               /*timing value(us):tW,tDP,tBP,tPP,tSE,tBE32,tBE,tCE,tWREAW*/
                               30000,10,100,10000,240000,3000000,3500000,240000000,1
                               )

        },

};

/*
 * Function:      MxSoftwareInit
 * Arguments:      Mxic,          pointer to an mxchip structure of nor flash device.
 *                EffectiveAddr, base address.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for initializing the variables.
 */
int MxSoftwareInit(MxChip *Mxic, u32 EffectiveAddr)
{
    // int Status;
    static    MxSpi Spi  ;
    Mxic->Priv = &Spi;

    /*
     * If the device is busy, disallow the initialize and return a status
     * indicating it is already started. This allows the user to stop the
     * device and re-initialize, but prevents a user from inadvertently
     * initializing. This assumes the busy flag is cleared at startup.
     */
    if (Spi.IsBusy == TRUE) {
        return MXST_DEVICE_IS_STARTED;
    }

    /*
     * Set some default values.
     */
    Spi.IsBusy = FALSE;
    Spi.BaseAddress = EffectiveAddr;
    Spi.SendBufferPtr = NULL;
    Spi.RecvBufferPtr = NULL;
    Spi.RequestedBytes = 0;
    Spi.RemainingBytes = 0;
    Spi.IsReady = MX_COMPONENT_IS_READY;
    Spi.CurMode = MODE_SPI;
    Spi.CurAddrMode = SELECT_3B;
    Spi.FlashProtocol = PROT_1_1_1;
    Spi.PreambleEn = FALSE;
    Spi.DataPass = FALSE;
//    Spi.SopiDqs = TRUE;
    Spi.SopiDqs = FALSE;
    Spi.HardwareMode = IOMode;

    return MXST_SUCCESS;
}

/*
 * Function:      MxIdMatch
 * Arguments:      Mxic:     pointer to an mxchip structure of nor flash device.
 *                Id:       data buffer to store the ID value.
 * Return Value:  MXST_SUCCESS.
 *                MXST_ID_NOT_MATCH.
 * Description:   This function is for checking if the ID value is matched to the ID in flash list.
 *                If they are matched, flash information will be assigned to Mxic structure.
 */
static int MxIdMatch(MxChip *Mxic, u8 *Id)
{
    MxFlashInfo *FlashInfo;
    int n,m;

    for (n = 0; n < ARRAY_SIZE(SpiFlashParamsTable) ; n++) {
        FlashInfo = &SpiFlashParamsTable[n];
        if (!memcmp(Id, FlashInfo->Id, 3)) {
            for(m=0; m<SPI_NOR_FLASH_MAX_ID_LEN; m=m+1)
                Mxic->Id[m] = FlashInfo->Id[m];
            Mxic->ChipSupMode = FlashInfo->SupModeFlag;
            Mxic->ChipSpclFlag = FlashInfo->SpclFlag;
            Mxic->PageSz = FlashInfo->PageSz;
            Mxic->BlockSz = FlashInfo->BlockSz;
            Mxic->N_Blocks = FlashInfo->N_Blocks;
            Mxic->ChipSz = Mxic->BlockSz * Mxic->N_Blocks;
            Mxic->RdDummy = FlashInfo->RdDummy;
            Mxic->SPICmdList = FlashInfo->SPICmdList;
            Mxic->QPICmdList = FlashInfo->QPICmdList;
            Mxic->OPICmdList = FlashInfo->OPICmdList;
            Mxic->tW = FlashInfo->tW;
            Mxic->tDP = FlashInfo->tDP;
            Mxic->tBP = FlashInfo->tBP;
            Mxic->tPP = FlashInfo->tPP;
            Mxic->tSE = FlashInfo->tSE;
            Mxic->tBE32 = FlashInfo->tBE32;
            Mxic->tBE = FlashInfo->tBE;
            Mxic->tCE = FlashInfo->tCE;
            Mxic->WriteBuffStart = FALSE;
            for(m=0; m<PASSWORD_LEN; m=m+1)
                Mxic->Pwd[m] = PASSWORD_INIT_VALUE;

            return MXST_SUCCESS;
        }
    }
    return MXST_ID_NOT_MATCH;
}

/*
 * Function:      MxScanMode
 * Arguments:      Mxic:      pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for getting the current mode of flash by reading and matching ID value in different mode.
 */
int MxScanMode(MxChip *Mxic)
{
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u8 Id[SPI_NOR_FLASH_MAX_ID_LEN];

    Mx_printf("\tStart matching the device ID\r\n", 0);
    /*
     * Standard SPI
     */
    Spi->CurMode = MODE_SPI;
    Status = MxRDID(Mxic, 3, Id);
    if (Status != MXST_SUCCESS)
        return Status;
//    Status = MxRDID(Mxic, 3, Id);
//    if (Status != MXST_SUCCESS)
//        return Status;
//    Id[0] =0xC2;Id[1] =0x81;Id[2] =0x3a;

    Mx_printf("\t\t SPI, ID: %02X%02X%02X\r\n", Id[0], Id[1], Id[2]);
    Status = MxIdMatch(Mxic, Id);
    if (Status == MXST_SUCCESS)
        return Status;

    /*
     * OCTA DOPI
     */
    Spi->CurMode = MODE_DOPI;
    Status = MxRDID(Mxic, 3, Id);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("\t\tDOPI, ID: %02X%02X%02X\r\n", Id[0], Id[1], Id[2]);
    Status = MxIdMatch(Mxic, Id);
    if (Status == MXST_SUCCESS)
        return Status;

    /*
     * OCTA SOPI
     */
    Spi->CurMode = MODE_SOPI;
    Status = MxRDID(Mxic, 3, Id);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("\t\tSOPI, ID: %02X%02X%02X\r\n", Id[0], Id[1], Id[2]);
    Status = MxIdMatch(Mxic, Id);
    if (Status == MXST_SUCCESS)
        return Status;

    /*
     *  QPI
     */
    Spi->CurMode = MODE_QPI;
    Status = MxQPIID(Mxic, 3, Id);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("\t\t QPI, ID: %02X%02X%02X\r\n", Id[0], Id[1], Id[2]);
    Status = MxIdMatch(Mxic, Id);
    if (Status == MXST_SUCCESS)
        return Status;

    Mx_printf("\t@warning: device ID match failed\r\n", 0);
    return MXST_ID_NOT_MATCH;
}

 /*
  * Function:      MxRdDmyWRCR
  * Arguments:       Mxic:      pointer to an mxchip structure of nor flash device.
  * Return Value:  MXST_SUCCESS
  *                MXST_FAILURE
  * Description:   This function is for setting dummy cycle in different read mode by writing configuration register.
  */
 int MxRdDmyWRCR(MxChip *Mxic)
 {
    u8 Cr[2];
    u8 Sr[2];
    u8 Status,RdProt;
    u8 IsCrBit7, IsCrBit6;
    MxSpi *Spi = Mxic->Priv;

    if (Mxic->ChipSupMode & MODE_DOPI) {
        Status = MxRDCR2(Mxic, CR2_DC_ADDR, Cr);
        if (Status != MXST_SUCCESS)
            return Status;
    }
    else{
        Status = MxRDCR(Mxic, Cr);
        if (Status != MXST_SUCCESS)
            return Status;
    }

    switch (Spi->CurMode)
    {
        case  MODE_FS_READ:
        case MODE_FSDT_READ:
            RdProt = 0;
            break;
        case MODE_DUAL_READ:
            RdProt = 1;
            break;
        case MODE_DUAL_IO_READ:
        case MODE_DUAL_IO_DT_READ:
            RdProt = 2;
            break;
        case MODE_QUAD_READ:
            RdProt = 3;
            break;
        case MODE_QUAD_IO_READ:
        case MODE_QUAD_IO_DT_READ:
            RdProt = 4;
            break;
        case MODE_QPI:
            RdProt = 5;
            break;
        case MODE_SOPI:
        case MODE_DOPI:
            RdProt = 6;
            break;
        default:
            return MXST_FAILURE;
            break;
    }

    if (Mxic->RdDummy[RdProt].CRValue != 0xFF) {
        if (Mxic->ChipSupMode & MODE_DOPI) {
            Cr[0] &= 0xF8;
            Cr[0] |= (Mxic->RdDummy[RdProt].CRValue & 0x03);
            Status = MxWRCR2(Mxic, CR2_DC_ADDR, Cr);
            if (Status != MXST_SUCCESS)
                return Status;
        }
        else{
            Status = MxRDSR(Mxic, Sr);
            if (Status != MXST_SUCCESS)
                return Status;
            IsCrBit7 = ((Mxic->RdDummy[RdProt].CRValue & 0x0F) == 0x0F);
            IsCrBit6 = ((Mxic->RdDummy[RdProt].CRValue & 0xF0) == 0xF0);
            if (IsCrBit7) {
                Cr[0] &= 0x7F;
                Cr[0] |= (Mxic->RdDummy[RdProt].CRValue & 0x10) << 3;
            } else if (IsCrBit6) {
                Cr[0] &= 0xBF;
                Cr[0] |= (Mxic->RdDummy[RdProt].CRValue & 0x01) << 6;
            } else {
                Cr[0] &= 0x3F;
                Cr[0] |= (Mxic->RdDummy[RdProt].CRValue & 0x03) << 6;
            }
            Sr[1] = Cr[0];
            Status = MxWRSR(Mxic, Sr);
            if (Status != MXST_SUCCESS)
                return Status;
        }
        return Mxic->RdDummy[RdProt].Dummy;
    }
    else if(Mxic->RdDummy[RdProt].Dummy != 0xFF) {
        return Mxic->RdDummy[RdProt].Dummy;
    }
    return MXST_SUCCESS;
 }

 /*
  * Function:      MxChangeMode
  * Arguments:       Mxic,        pointer to an mxchip structure of nor flash device.
  *                SetMode,     variable in which to store the operation mode to set.
  *                SetAddrMode, variable in which to store the address mode to set.
  * Return Value:  MXST_SUCCESS.
  *                MXST_FAILURE.
  * Description:   This function is for changing the operation mode and address mode.
  */
 int MxChangeMode(MxChip *Mxic, u32 SetMode,u32 SetAddrMode)
{
    int Status;
    MxSpi *Spi = Mxic->Priv;
    u32 SupMode = Mxic->ChipSupMode;
    static u8 FirstFlag = 0;
    u8 CR2Value[2] = {0};

    if(SetAddrMode == ADDR_MODE_AUTO_SEL)
    {
        /*
         * if bigger than 128Mb,select 4 byte address
         */
        if( Mxic->ChipSz > 0xFFFFFF)
        {
            Spi->CurAddrMode = SELECT_4B;
            if((Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_4B) && (!(Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD)))
            {
                Status = MxEN4B(Mxic);
                if (Status == MXST_SUCCESS)
                    return Status;
            }
        }
        else
            Spi->CurAddrMode = SELECT_3B;
    }
    else
    {
        if(( Mxic->ChipSz < 0xFFFFFF) && (SetAddrMode == SELECT_4B))
        {
            Mx_printf("Flash is only supported for 3 byte address mode\r\n");
            Spi->CurAddrMode = SELECT_3B;
        }
        else if((Mxic->ChipSz > 0xFFFFFF) && (!(Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_4B)) && (!(Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD)) && (SetAddrMode == SELECT_3B))
        {
            Mx_printf("Flash is only supported for 4 byte address mode\r\n");
            Spi->CurAddrMode = SELECT_4B;
        }

        else if((Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_4B) && (!(Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD)))
        {
            Mx_printf("Flash is supported for 3 byte and 4 byte address mode,Change by EN4B/EX4B\r\n");
            if(SetAddrMode == SELECT_4B)
            {
                Status = MxEN4B(Mxic);
                if (Status == MXST_SUCCESS)
                    return Status;
            }
            else
            {
                Status = MxEX4B(Mxic);
                if (Status == MXST_SUCCESS)
                    return Status;
            }
            Spi->CurAddrMode = SetAddrMode;
        }
        else
            Spi->CurAddrMode = SetAddrMode;
    }


    if ((Spi->CurMode == SetMode)&&(FirstFlag))
    {
        Mx_printf("\t\tFlash does not need change mode\r\n");
        return MXST_SUCCESS;
    }

    /*********************�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{************************
    **********************�x         OCTA R/W/E           �x************************
    **********************�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}***********************/
    /*
     * Change to DOPI mode
     */

    if (SetMode & MODE_DOPI) {
        if (!(SupMode & MODE_DOPI)) {
            Mx_printf("\t\tFlash is not supported for DOPI mode\r\n");

            Mx_printf("\t\tChange to [Standard SPI] mode\r\n");
            if (Spi->CurMode & MODE_QPI) {
                Status = MxRSTQPI(Mxic);
                if (Status != MXST_SUCCESS)
                    return Status;
             }
            Spi->CurMode = MODE_SPI;
            Spi->FlashProtocol = PROT_1_1_1;

            Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B)? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
            Mxic->AppGrp._Erase = (Spi->CurAddrMode==SELECT_3B) ? MxBE : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxBE4B : MxBE);
        } else {
            if (Spi->CurMode != MODE_SPI) {
                CR2Value[0] = CR2_SPI_EN;
                Status = MxWRCR2(Mxic, CR2_OPI_EN_ADDR, CR2Value);
                if (Status != MXST_SUCCESS)
                    return Status;
                Spi->CurMode = MODE_SPI;
                Spi->FlashProtocol = PROT_1_1_1;
            }
            CR2Value[0] = CR2_DOPI_EN;
            Status = MxWRCR2(Mxic, CR2_OPI_EN_ADDR, CR2Value);
            if (Status != MXST_SUCCESS)
                return Status;
            Mx_printf("\t\tChange Mode To [DOPI]\r\n");
            Spi->CurMode = MODE_DOPI;
            Spi->FlashProtocol = PROT_8D_8D_8D;
            Mxic->AppGrp._Read = Mx8DTRD;
            Mxic->AppGrp._Write = Mx8DTRPP;
            Mxic->AppGrp._Erase = Mx8DTRBE;
        }
    }

    /*
     * Change to SOPI mode
     */
    else if (SetMode & MODE_SOPI) {
        if (!(SupMode & MODE_SOPI)) {
            Mx_printf("\t\tFlash is not supported for SOPI mode\r\n");

            Mx_printf("\t\tChange to [Standard SPI] mode\r\n");
            if (Spi->CurMode & MODE_QPI) {
                Status = MxRSTQPI(Mxic);
                if (Status != MXST_SUCCESS)
                    return Status;
             }
            Spi->CurMode = MODE_SPI;
            Spi->FlashProtocol = PROT_1_1_1;
            Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
            Mxic->AppGrp._Erase = (Spi->CurAddrMode==SELECT_3B) ? MxBE : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxBE4B : MxBE);
        } else {
            if (Spi->CurMode != MODE_SPI) {
                CR2Value[0] = CR2_SPI_EN;
                Status = MxWRCR2(Mxic, CR2_OPI_EN_ADDR, CR2Value);
                if (Status != MXST_SUCCESS)
                    return Status;
                Spi->CurMode = MODE_SPI;
                Spi->FlashProtocol = PROT_1_1_1;
            }
            if(Spi->SopiDqs)
                CR2Value[0] = CR2_SOPI_DQS_EN;
            else
                CR2Value[0] = 0;
            Status = MxWRCR2(Mxic, CR2_DQS_EN_ADDR, CR2Value);
            if (Status != MXST_SUCCESS)
                return Status;
            CR2Value[0] = CR2_SOPI_EN;
            Status = MxWRCR2(Mxic, CR2_OPI_EN_ADDR, CR2Value);
            if (Status != MXST_SUCCESS)
                return Status;


            Mx_printf("\t\tChange Mode To [SOPI]\r\n");
            Spi->CurMode = MODE_SOPI;
            Spi->FlashProtocol = PROT_8_8_8;
            Mxic->AppGrp._Read = Mx8READ;
            Mxic->AppGrp._Write = Mx8PP;
            Mxic->AppGrp._Erase = Mx8BE;
        }
    }
    /*********************�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{************************
    **********************�x          QPI R/W/E           �x************************
    **********************�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}***********************/
    /*
     * Change to QPI mode
     */
    else if (SetMode & MODE_QPI) {
        if (!(SupMode & MODE_QPI)) {
            Mx_printf("\t\tFlash is not supported for QPI mode\r\n");
            Mx_printf("\t\tChange to [Standard SPI] mode\r\n");
             if (Spi->CurMode & (MODE_OPI)) {
                CR2Value[0] = CR2_SPI_EN;
                Status = MxWRCR2(Mxic, CR2_OPI_EN_ADDR, CR2Value);
                if (Status != MXST_SUCCESS)
                    return Status;
             }
            Spi->CurMode = MODE_SPI;
            Spi->FlashProtocol = PROT_1_1_1;
            Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP :((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
            Mxic->AppGrp._Erase = (Spi->CurAddrMode==SELECT_3B) ? MxBE :((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxBE4B : MxBE);

        } else {
            Status = MxEQPI(Mxic);
            if (Status != MXST_SUCCESS)
                return Status;
            Mx_printf("\t\tChange Mode To [QPI]\r\n");
            Spi->CurMode = MODE_QPI;
            Spi->FlashProtocol = PROT_4_4_4;
            Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? Mx4READ : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? Mx4READ4B : Mx4READ);
            Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
            Mxic->AppGrp._Erase = (Spi->CurAddrMode==SELECT_3B) ? MxBE : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxBE4B : MxBE);
        }
    }
    /*********************�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{************************
    **********************�x          SPI R/W/E           �x************************
    **********************�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}***********************/
    /*
        * Change to SPI mode
    */
    else{

         if (Spi->CurMode & (MODE_OPI)) {
            CR2Value[0] = CR2_SPI_EN;
            Status = MxWRCR2(Mxic, CR2_OPI_EN_ADDR, CR2Value);
            if (Status != MXST_SUCCESS)
                return Status;
         } else if (Spi->CurMode & MODE_QPI) {
            Status = MxRSTQPI(Mxic);
            if (Status != MXST_SUCCESS)
                return Status;
         }

        /**********************�x          SPI Read            �x************************/
        /*
         * Change to SPI Quad IO Read mode
         */
         if (SetMode & MODE_QUAD_IO_READ) {
            if (!(SupMode & MODE_QUAD_IO_READ)) {
                Mx_printf("\t\tFlash is not supported for [Quad IO Read(144)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [Quad IO Read(144)] mode\r\n");
                Spi->CurMode = MODE_QUAD_IO_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? Mx4READ : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? Mx4READ4B : Mx4READ);
            }
        }
        else if (SetMode & MODE_QUAD_READ) {
            /*
             * Change to SPI Quad Read mode
             */
            if (!(SupMode & MODE_QUAD_READ)) {
                Mx_printf("\t\tFlash is not supported for [Quad Read(114)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [Quad Read(114)] mode\r\n");
                Spi->CurMode = MODE_QUAD_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxQREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxQREAD4B : MxQREAD);
            }
        }
        else if (SetMode & MODE_DUAL_IO_READ) {
            /*
             * Change to SPI Dual IO Read mode
             */
            if (!(SupMode & MODE_DUAL_IO_READ)) {
                Mx_printf("\t\tFlash is not supported for [Dual IO Read(122)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [Dual IO Read(122)] mode\r\n");
                Spi->CurMode = MODE_DUAL_IO_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? Mx2READ : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? Mx2READ4B : Mx2READ);
            }
        }
        else if (SetMode & MODE_DUAL_READ) {
            /*
             * Change to SPI Dual Read mode
             */
            if (!(SupMode & MODE_DUAL_READ)) {
                Mx_printf("\t\tFlash is not supported for [Dual Read(112)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [Dual Read(112)] mode\r\n");
                Spi->CurMode = MODE_DUAL_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxDREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxDREAD4B : MxDREAD);
            }
        }
        else if (SetMode & MODE_FS_READ) {
            /*
             * Change to SPI Fast Read mode
             */
            if (!(SupMode & MODE_FS_READ)) {
                Mx_printf("\t\tFlash is not supported for [Fast Read(111)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [Fast Read(111)] mode\r\n");
                Spi->CurMode = MODE_FS_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxFASTREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxFASTREAD4B : MxFASTREAD);
            }
        }
        else if (SetMode & MODE_FSDT_READ) {
            /*
             * Change to SPI FastDT Read mode
             */
            if (!(SupMode & MODE_FSDT_READ)) {
                Mx_printf("\t\tFlash is not supported for [FastDT Read(111)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [FastDT Read(1-1D-1D)] mode\r\n");
                Spi->CurMode = MODE_FSDT_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxFASTDTRD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxFASTDTRD4B : MxFASTDTRD);
            }
        }
        else if (SetMode & MODE_DUAL_IO_DT_READ) {
            /*
             * Change to SPI Dual IO DT Read mode
             */
            if (!(SupMode & MODE_DUAL_IO_DT_READ)) {
                Mx_printf("\t\tFlash is not supported for [Dual IO DT Read(122)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [Dual IO DT Read(1-2D-2D)] mode\r\n");
                Spi->CurMode = MODE_DUAL_IO_DT_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? Mx2DTRD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? Mx2DTRD4B : Mx2DTRD);
            }
        }
        else if (SetMode & MODE_QUAD_IO_DT_READ) {
            /*
             * Change to SPI QUAD IO DT Read mode
             */
            if (!(SupMode & MODE_QUAD_IO_DT_READ)) {
                Mx_printf("\t\tFlash is not supported for [QUAD IO DT Read(144)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Read] mode\r\n");
                Spi->CurMode = MODE_SPI;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
            } else {
                Mx_printf("\t\tChange to [QUAD IO DT Read(1-4D-4D)] mode\r\n");
                Spi->CurMode = MODE_QUAD_IO_DT_READ;
                Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? Mx4DTRD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? Mx4DTRD4B : Mx4DTRD);
            }
        }
        else {
            Mx_printf("\t\tChange Mode To [Standard SPI Read]\r\n");
            Spi->CurMode = MODE_SPI;
            Mxic->AppGrp._Read = (Spi->CurAddrMode==SELECT_3B) ? MxREAD : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxREAD4B : MxREAD);
        }
         Spi->FlashProtocol = PROT_1_1_1;
        /*********************�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{************************
        **********************�x          SPI Write           �x************************
        **********************�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}***********************/
        /*
         * Change to SPI Quad IO Write mode
         */
        if (SetMode & MODE_QUAD_IO_WRITE) {
            if (!(SupMode & MODE_QUAD_IO_WRITE)) {
                Mx_printf("\t\tFlash is not supported for [Quad IO Write(144)] mode\r\n");
                Mx_printf("\t\tChange to [SPI Normal Write] mode\r\n");
                Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
            } else {
                Mx_printf("\t\tChange to [Quad IO Write(144)] mode\r\n");
                Spi->CurMode |= MODE_QUAD_IO_WRITE;
                Spi->FlashProtocol = PROT_1_4_4;
                Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? Mx4PP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? Mx4PP4B : Mx4PP);
            }

        } else {
            Mx_printf("\t\tChange Mode To [Standard SPI Write]\r\n");
            //Spi->CurMode = MODE_SPI;
            Mxic->AppGrp._Write = (Spi->CurAddrMode==SELECT_3B) ? MxPP : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxPP4B : MxPP);
        }

        Mxic->AppGrp._Erase = (Spi->CurAddrMode==SELECT_3B) ? MxBE : ((Mxic->SPICmdList[MX_RD_CMDS] & MX_4B_RD) ? MxBE4B : MxBE);
    }
    /* setup read dummy */
    Spi->LenDummy = MxRdDmyWRCR(Mxic);

    if((Spi->CurMode != MODE_QPI) & (Spi->CurMode != MODE_SOPI) & (Spi->CurMode != MODE_DOPI))
        Spi->FlashProtocol = PROT_1_1_1;

    if(!FirstFlag)
        FirstFlag = 1;

    return MXST_SUCCESS;
}

#ifdef BLOCK2_SERCURITY_OTP
 /*
   * Function:      MxQryLockMode
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   * Return Value:  MXST_FAILURE.
   *                 BP_MODE.
   *                ASP_SOLID_MODE.
   *                ASP_PWD_MODE.
   *                SBP_MODE
   * Description:   This function is for getting protection mode.
   */
   int MxQryLockMode(MxChip *Mxic)
  {
      int Status;
      u8 SecurReg;
       u8 LockReg[2];

       if (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_RDSCUR) {
        Status = MxRDSCUR(Mxic, &SecurReg);
        if (Status != MXST_SUCCESS)
            return Status;
        //Mx_printf("SecurReg : %02X\r\n", SecurReg);
        if (!(SecurReg & WPSEL_MASK))
        {
            Mx_printf("\t\tCurrent mode is BP Mode\r\n", 0);
              return BP_MODE;
        }
       }
       else
       {
           Mx_printf("\t\tCurrent mode is BP Mode\r\n", 0);
           return BP_MODE;
       }

       if (Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_SBLK) {
           Mx_printf("\t\tCurrent mode is Single Block Protection Mode\r\n", 0);
           return SBP_MODE;
       }

    Status = MxRDLR(Mxic, LockReg);
    if (Status != MXST_SUCCESS)
        return Status;
    //Mx_printf("LockReg : %02X\r\n", LockReg[0]);
      if (!(LockReg[0] & LR_PWDP_MASK))
      {
          Mx_printf("\t\tCurrent mode is ASP password Mode\r\n", 0);
          return ASP_PWD_MODE;
      }

      Mx_printf("\t\tCurrent mode is ASP solid Mode\r\n", 0);
      return ASP_SOLID_MODE;
  }

  /*
   * Function:      MxSetBoundary
   * Arguments:        Mxic,     pointer to an mxchip structure of nor flash device.
   *                ofs_s,    start address of area to lock or unlock.
   *                ofs_d,    end address of area to lock or unlock.
   *                islocked, to lock or to unlock flash.
   * Return Value:  MXST_SUCCESS.
   *                MXST_FAILURE.
   * Description:   This function is for cutting offset to boundary point.
   */
  int MxSetBoundary(MxChip *Mxic, u32 *ofs_s, u32 *ofs_e, u8 islocked)
  {
      u32 ofs, bdy_size, mod, wp64k_first, wp64k_last;

      if (*ofs_s >= Mxic->ChipSz)
          return MXST_FAILURE;

      if (*ofs_e >= Mxic->ChipSz)
          *ofs_e = Mxic->ChipSz - 1;

      /* check ofs */
      ofs = *ofs_s;
      wp64k_first = MX_WP64K_FIRST;
      wp64k_last = MX_WP64K_LAST(Mxic-> ChipSz);

      bdy_size = (*ofs_s < wp64k_first || *ofs_s >= wp64k_last) ? MX_4KB : MX_64KB;
      mod = ofs % bdy_size;
      if (mod)
          *ofs_s += bdy_size - mod;

      ofs = *ofs_e;
      bdy_size = (*ofs_e <= wp64k_first || *ofs_e > wp64k_last) ? MX_4KB : MX_64KB;
      mod = ofs % bdy_size;
      if (mod && mod != (bdy_size - 1))
          *ofs_e -= mod + 1;
      else if (!mod)
          (*ofs_e)--;

      if ((*ofs_s > *ofs_e && !islocked) || *ofs_s >= Mxic-> ChipSz || (*ofs_e) >= Mxic-> ChipSz )
          return MXST_FAILURE;

      return MXST_SUCCESS;
  }


 /*
  * Function:      MxGetLockedRange
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Sr,    the value of status register.
  *                TB,    the value of TB bit in configuration register.
  *                Addr,  the start address of locked area.
  *                Len,   bytes number of locked area.
  * Return Value:  NONE.
  * Description:   This function is for getting the range of locked area by the value of status register and configuration register.
  */
 static void MxGetLockedRange(MxChip *Mxic, u8 Sr, u8 TB, u32 *Addr, u64 *Len)
 {
     u8 Mask = SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;
     int Shift = ffs(Mask) - 1;

     if (!(Sr & Mask)) {
         /* No protection */
         *Addr = 0;
         *Len = 0;
     } else {
         *Len  = pow(2,((Sr & Mask) >> Shift) - 1) * 64 * 1024;
         if(!TB)
             *Addr = Mxic->ChipSz - *Len;
         else
             *Addr = 0;
     }
 }

 /*
  * Function:      MxIsLockedSr
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  the start address of checking area.
  *                Len,   bytes number of checking area.
  *                Sr,    the value of status register.
  * Return Value:  MXST_FAILURE.
  *                FLASH_IS_LOCKED.
  *                FLASH_IS_UNLOCKED.
  * Description:   This function is for checking if the area is locked by the value of status register and configuration register.
  */
 static int MxIsLockedSr(MxChip *Mxic, u32 Addr, u64 Len,u8 Sr)
 {
    u32 LockOffs;
     u64 LockLen;
     u8  Cr, Status;

    if((Mxic->ChipSpclFlag & SUPPORT_WRSR_CR) && (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_RDCR))
    {
        Status = MxRDCR(Mxic, &Cr);
        if (Status != MXST_SUCCESS)
            return Status;
    }

     MxGetLockedRange(Mxic, Sr, Cr & CR_TB_MASK, &LockOffs, &LockLen);

     if((Addr + Len <= LockOffs + LockLen) && (Addr >= LockOffs))
         return FLASH_IS_LOCKED;
     else
         return FLASH_IS_UNLOCKED;
 }

 /*
  * Function:      MxLock
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  32 bit flash memory address of locked area.
  *                Len,   number of bytes to lock.
  * Return Value:  MXST_SUCCESS.
  *                MXST_FAILURE.
  * Description:   This function is for writing protection a specified block (or sector) of flash memory in BP protection mode
  *                by writing status register.
  */
  int MxLock(MxChip *Mxic, u32 Addr, u64 Len)
 {
     u8 Status;
     u8 SrOld, SrNew, Cr;
     u8 Sr[2];
     u32 ofs_s, ofs_e;
     u8 Mask = SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;
     u8 Shift = ffs(Mask) - 1 , Val;

     ofs_s = Addr;
     ofs_e = Addr + Len -1;

     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
     Addr = ofs_s;
     Len = ofs_e - Addr + 1;

     Status = MxRDSR(Mxic, &SrOld);
    if (Status != MXST_SUCCESS)
        return Status;

    if((Mxic->ChipSpclFlag & SUPPORT_WRSR_CR) && (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_RDCR))
    {
        Status = MxRDCR(Mxic, &Cr);
        if (Status != MXST_SUCCESS)
            return Status;
    }

    /*
     * T/B = 0 : lock from the last block(bottom)
     */
    if(!(Cr & CR_TB_MASK))
    {
        /* SPI NOR always locks to the end */
        if (Addr + Len != Mxic->ChipSz) {
            /* Does combined region extend to end? */
            if (!MxIsLockedSr(Mxic, Addr + Len, Mxic->ChipSz - Addr - Len, SrOld))
            {
                Mx_printf("combined region does not extend to end\r\n", 0);
                return MXST_FAILURE;
            }
            Len = Mxic->ChipSz - Addr;
        }
    }
    /*
     * T/B = 1 : lock from the first block(top)
     */
    else
    {
        /* SPI NOR always locks from the top */
        if (Addr != 0) {
            /* Does combined region from the top? */
            if (!MxIsLockedSr(Mxic, 0, Addr , SrOld))
            {
                Mx_printf("combined region does not from the top\r\n", 0);
                return MXST_FAILURE;
            }
            Len =  Addr + Len;
        }
    }

    Val = ( (int)log2(Len/64/1024) + 1 ) << Shift;

    if (Val & ~Mask)
        return MXST_FAILURE;
    /* Don't "lock" with no region! */
    if (!(Val & Mask))
        return MXST_FAILURE;

    SrNew = (SrOld & ~Mask) | Val;

    /* Only modify protection if it will not unlock other areas */
    if ((SrNew & Mask) <= (SrOld & Mask))
        return MXST_FAILURE;

    Sr[0] = SrNew;
    Sr[1] = Cr;
    Status = MxWRSR(Mxic, &Sr[0]);
    if (Status != MXST_SUCCESS)
        return Status;
    return  Status;
 }

  /*
   * Function:      MxUnlock
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   *                Addr,  32 bit flash memory address of unlocked area.
   *                Len,   number of bytes to unlock.
   * Return Value:  MXST_SUCCESS.
   *                MXST_FAILURE.
   * Description:   This function is for canceling the block (or sector)  write protection state in BP protection mode
   *                by writing status register.
   */
 int MxUnlock(MxChip *Mxic, u32 Addr, u64 Len)
 {
     u8 Status;
     u8 SrOld, SrNew, Cr;
     u8 Sr[2];
     u32 ofs_s, ofs_e;
     u8 Mask = SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;
     u8 Shift = ffs(Mask) - 1, Val;

     ofs_s = Addr;
     ofs_e = Addr + Len -1;

     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
     Addr = ofs_s;
     Len = ofs_e - Addr + 1;

     Status = MxRDSR(Mxic, &SrOld);
    if (Status != MXST_SUCCESS)
        return Status;

    if((Mxic->ChipSpclFlag & SUPPORT_WRSR_CR) && (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_RDCR))
    {
        Status = MxRDCR(Mxic, &Cr);
        if (Status != MXST_SUCCESS)
            return Status;
    }

    /* Cannot unlock; would unlock larger region than requested */
    if ((!(Cr & CR_TB_MASK)) && MxIsLockedSr(Mxic, Addr - Mxic->BlockSz, Mxic->BlockSz, SrOld))
    {
        Mx_printf("@ERR: Cannot unlock; would unlock larger region than requested\r\n", 0);
        return MXST_FAILURE;
    }
    else if ((Cr & CR_TB_MASK) && MxIsLockedSr(Mxic, Addr + Len, Mxic->BlockSz, SrOld))
    {
        Mx_printf("@ERR: Cannot unlock; would unlock larger region than requested\r\n", 0);
        return MXST_FAILURE;
    }

    Len = Mxic->ChipSz - (Addr + Len);

    if(Len > 0)
        Val = ( (int)log2(Len/64/1024) + 1 ) << Shift;
    else
        Val = 0;

    SrNew = (SrOld & ~Mask) | Val;

    /* Only modify protection if it will not lock other areas */
    if ((SrNew & Mask) >= (SrOld & Mask))
        return MXST_FAILURE;

    Sr[0] = SrNew;
    Sr[1] = Cr;
    return  MxWRSR(Mxic, &Sr[0]);
 }

 /*
  * Function:      MxIsLocked
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  32 bit flash memory address of checking area.
  *                Len,   number of bytes to check.
  * Return Value:  MXST_FAILURE
  *                FLASH_IS_UNLOCKED.
  *                FLASH_IS_LOCKED.
  * Description:   This function is for checking if the block (or sector) is locked in BP protection mode.
  */
  int MxIsLocked(MxChip *Mxic, u32 Addr, u64 Len)
 {
     u8 Sr;
     u8 Status;

     Status = MxRDSR(Mxic, &Sr);
    if (Status != MXST_SUCCESS)
        return Status;
     return MxIsLockedSr(Mxic, Addr, Len, Sr);
 }


  /*
   * Function:      MxIsSpbBitLocked
   * Arguments:        Mxic:  pointer to an mxchip structure of nor flash device.
   * Return Value:  MXST_FAILURE.
   *                MXST_DEVICE_SPB_IS_LOCKED : SPB is locked down(SPB lock down bit is 0).
   *                MXST_DEVICE_SPB_ISNOT_LOCKED : SPB is not locked down(SPB lock down bit is 1).
   * Description:   This function is for checking if SPB bit is locked down by reading SPBLK bit.
   */
  int MxIsSpbBitLocked(MxChip *Mxic)
  {
      int Status;
      u8 SpbLKB;

      if (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_SPBLK)
      {
          Status = MxRDSPBLK(Mxic, &SpbLKB);
          if (Status != MXST_SUCCESS)
              return Status;
          Mx_printf("\t\tSPB Lock Register : %02X\r\n", SpbLKB);
          if (SpbLKB & SPBLR_SPBLB_MASK)
          {
              Mx_printf("\t\tSPB is not locked down,SPB can be changed\r\n", 0);
              return MXST_DEVICE_SPB_ISNOT_LOCKED;
          }
          else
          {
              Mx_printf("\t\tSPB is locked down,SPB cannot be changed\r\n", 0);
              return MXST_DEVICE_SPB_IS_LOCKED;
          }
      }
      else
      {
          Status = MxRDLR(Mxic, &SpbLKB);
          if (Status != MXST_SUCCESS)
              return Status;
          Mx_printf("\t\tLock Register : %02X\r\n", SpbLKB);
          if (SpbLKB & LR_SPBLB_MASK)
          {
              Mx_printf("\t\tSPB is not locked down,SPB can be changed\r\n", 0);
              return MXST_DEVICE_SPB_ISNOT_LOCKED;
          }
          else
          {
              Mx_printf("\t\tSPB is locked down,SPB cannot be changed\r\n", 0);
              return MXST_DEVICE_SPB_IS_LOCKED;
          }
      }
  }

  /*
   * Function:      MxSpbBitLockDown
   * Arguments:        Mxic:  pointer to an mxchip structure of nor flash device.
   * Return Value:  MXST_SUCCESS.
   *                MXST_FAILURE.
   * Description:   This function is for is setting SPB lock.
   */
  int MxSpbBitLockDown(MxChip *Mxic)
  {
      int Status;
      u8 LockReg[2];

      if (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_SPBLK)
      {
          Status = MxSPBLK(Mxic);
          if (Status != MXST_SUCCESS)
              return Status;
      }
      else
      {
          Status = MxRDLR(Mxic, LockReg);
          if (Status != MXST_SUCCESS)
              return Status;
          LockReg[0] &= LR_SPBLB_LOCK_EN;
          Status = MxWRLR(Mxic, LockReg);
          if (Status != MXST_SUCCESS)
              return Status;
      }

      return Status;
  }
  /*
   * Function:      MxAspIsLockedCheck
   * Arguments:        Mxic,     pointer to an mxchip structure of nor flash device.
   *                ofs_s,    start address of checking area.
   *                ofs_d,    end address of checking area.
   *                ASP_MODE, advanced sector protection mode.
   * Return Value:  MXST_FAILURE.
   *                 FLASH_IS_UNLOCKED.
   *                FLASH_IS_LOCKED.
   *                FLASH_IS_PARTIALLY_LOCKED
   * Description:   This function is for checking if the block (or sector) is locked in advanced sector protection mode.
   */
static int MxAspIsLockedCheck(MxChip *Mxic, u32 ofs_s, u32 ofs_e, enum MX_ASP_MODE ASP_MODE)
{
    u32 bdy_size, ofs_s_org = ofs_s, wp64k_first = MX_WP64K_FIRST, wp64k_last = MX_WP64K_LAST(Mxic-> ChipSz);
    u8 val_old = 0, val = 0;
    int Status;

    for (val = 0; ofs_s <= ofs_e; ofs_s += bdy_size) {
        bdy_size = (ofs_s < wp64k_first || ofs_s >= wp64k_last) ? MX_4KB : MX_64KB;

        switch (ASP_MODE) {
        case ASP_SPB:    Status = MxRDSPB(Mxic, ofs_s, &val);
                        if (Status != MXST_SUCCESS)
                            return Status;
                        break;
        case ASP_DPB:    Status = MxRDDPB(Mxic, ofs_s, &val);
                        if (Status != MXST_SUCCESS)
                            return Status;
                        break;
        case ASP_SB:    Status = MxRDBLOCK(Mxic, ofs_s, &val);
                        if (Status != MXST_SUCCESS)
                            return Status;
                        break;
        //case ASP_BWL:    ret = spi_nor_rdbwl(Mxic, ofs_s, &val);
//                        if (Status != MXST_SUCCESS)
//                            return Status;
//                        break;
        //case ASP_PERM:ret = spi_nor_rdplock(Mxic, ofs_s, &val);
//                        if (Status != MXST_SUCCESS)
//                            return Status;
//                        break;
        default: return MXST_FAILURE;
        }


        if (ofs_s == ofs_s_org)
            val_old = val;

        if (val_old != val)
            return FLASH_IS_PARTIALLY_LOCKED;
    }
    return !val ? FLASH_IS_UNLOCKED : FLASH_IS_LOCKED;
}

    /*
     * Function:      MxAspModeSet
     * Arguments:      Mxic,     pointer to an mxchip structure of nor flash device.
     *                ofs_s,    start address of area to lock or unlock.
     *                ofs_d,    end address of area to lock or unlock.
     *                ASP_MODE, advanced sector protection mode.
     * Return Value:  MXST_SUCCESS.
     *                MXST_FAILURE.
     * Description:   This function is for lock or unlock flash in advanced sector protection mode.
     *                It is only for DPB lock/unlock and SPB lock.
     */
 static int MxAspModeSet(MxChip *Mxic, u32 ofs_s, u32 ofs_e, enum MX_ASP_MODE ASP_MODE)
{
    u8 val_lock = 0xFF, val_unlock = 0x00;
    int Status = 0;
    u32 bdy_size, wp64k_first = MX_WP64K_FIRST, wp64k_last = MX_WP64K_LAST(Mxic-> ChipSz);

    for (; ofs_s <= ofs_e; ofs_s += bdy_size) {
        bdy_size = (ofs_s < wp64k_first || ofs_s >= wp64k_last) ? MX_4KB : MX_64KB;

        switch (ASP_MODE) {
            case ASP_DPB_LOCK:        Status = MxWRDPB(Mxic, ofs_s, &val_lock);        break;
            case ASP_DPB_UNLOCK:    Status = MxWRDPB(Mxic, ofs_s, &val_unlock);        break;
            case ASP_SPB_LOCK:        Status = MxWRSPB(Mxic, ofs_s);                     break;
            case ASP_SBLK:            Status = MxSBLK(Mxic,ofs_s);                    break;
            case ASP_SBULK:            Status = MxSBULK(Mxic,ofs_s);                    break;
            //case ASP_PLOCK:       Status = spi_nor_plock(nor, ofs_s);                break;
            //case ASP_BWL_LOCK:    Status = spi_nor_bwlp(nor, ofs_s);                break;
            if (Status != MXST_SUCCESS)
                return Status;
            default:
                return MXST_FAILURE;
        }
    }
    return Status;
}
   /*
    * Function:      MxSpbLock
    * Arguments:     Mxic,  pointer to an mxchip structure of nor flash device.
    *                ofs_s,  start address of area to lock.
    *                ofs_d,  end address of area to lock.
    * Return Value:  MXST_SUCCESS.
    *                MXST_FAILURE.
    *                MXST_DEVICE_SPB_IS_LOCKED
    * Description:   This function is for writing protection a specified block (or sector) of flash memory in solid protection mode.
    */
  static int MxSpbLock(MxChip *Mxic, u32 ofs_s, u32 ofs_e)
  {
      if( MxIsSpbBitLocked(Mxic) == MXST_DEVICE_SPB_ISNOT_LOCKED)
      {
         return MxAspModeSet(Mxic, ofs_s, ofs_e, ASP_SPB_LOCK);
      }
      else
      {
         Mx_printf("\t\tSPB is locked down,Unlock SPB failed\r\n", 0);
         return MXST_DEVICE_SPB_IS_LOCKED;
      }
  }

  /*
   * Function:      MxSpbUnlock
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   *                ofs_s,  start address of area to unlock.
   *                ofs_d,  end address of area to unlock.
   * Return Value:  MXST_SUCCESS.
   *                MXST_FAILURE.
   *                MXST_DEVICE_SPB_IS_LOCKED.
   * Description:   This function will cancel the block (or sector)  write protection state in solid protection mode.
   */
 static int MxSpbUnlock(MxChip *Mxic, u32 ofs_s, u32 ofs_e)
 {
     if( MxIsSpbBitLocked(Mxic) == MXST_DEVICE_SPB_ISNOT_LOCKED)
     {
         /* erase spb */
          return MxESSPB(Mxic);
     }
     else
     {
         Mx_printf("\t\tSPB is locked down,Unlock SPB failed\r\n", 0);
         return MXST_DEVICE_SPB_IS_LOCKED;
     }
 }

 /*
  * Function:      MxSpbIsLocked
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                ofs_s,  start address of checking area.
  *                ofs_d,  end address of checking area.
  * Return Value:  MXST_FAILURE.
  *                FLASH_IS_UNLOCKED.
  *                FLASH_IS_LOCKED.
  * Description:   This function is for checking if the block (or sector) is locked in solid protection mode.
  */
 static int MxSpbIsLocked(MxChip *Mxic, u32 ofs_s, u32 ofs_e)
 {
       return MxAspIsLockedCheck(Mxic, ofs_s, ofs_e, ASP_SPB);
 }

 /*
  * Function:      MxAspLock
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  32 bit flash memory address of locked area in advanced sector protection mode.
  *                Len,   number of bytes to lock in advanced sector protection mode.
  * Return Value:  MXST_SUCCESS.
  *                MXST_FAILURE.
  * Description:   This function is for writing protection a specified block (or sector) of flash memory in advanced sector protection mode.
  */
   int MxAspLock(MxChip *Mxic, u32 Addr, u64 Len)
  {
      int Status, LockMode;
      u32 ofs_s, ofs_e;

      LockMode = MxQryLockMode(Mxic);
      if (LockMode == MXST_FAILURE)
          return LockMode;

      if (LockMode == BP_MODE)
          return MxLock(Mxic, Addr, Len);

      /* for SP and DP */
      ofs_s = Addr;
      ofs_e = Addr + Len -1;
      Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
      if (Status != MXST_SUCCESS)
          return Status;
      if (LockMode == ASP_SOLID_MODE)
      {
          Mx_printf( "\t\tLock flash in SPB mode by WRSPB command \r\n");
          return MxSpbLock(Mxic, ofs_s, ofs_e);
      }

      if (LockMode == ASP_PWD_MODE) {
          Mx_printf( "ERROR: please use 'PASSWORD LOCK SPB MODE' to lock SPBs with password protection mode\r\n");
          return MXST_FAILURE;
      }

      return Status;
  }

   /*
    * Function:      MxAspUnlock
    * Arguments:     Mxic,  pointer to an mxchip structure of nor flash device.
    *                Addr,  32 bit flash memory address of unlocked area in advanced sector protection mode.
    *                Len,   number of bytes to unlock in advanced sector protection mode.
    * Return Value:  MXST_SUCCESS.
    *                MXST_FAILURE.
    * Description:   This function is for canceling the block (or sector)  write protection state in advanced sector protection mode.
    */
   int MxAspUnlock(MxChip *Mxic, u32 Addr, u64 Len)
  {
      int Status, LockMode;
      u32 ofs_s, ofs_e;

      LockMode = MxQryLockMode(Mxic);
      if (LockMode == MXST_FAILURE)
          return LockMode;

      if (LockMode == BP_MODE)
          return MxUnlock(Mxic, Addr, Len);

      /* for SP and DP */
      ofs_s = Addr;
      ofs_e = Addr + Len -1;
      Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
      if (Status)
          return Status;
      if (LockMode == ASP_SOLID_MODE)
      {
          Mx_printf( "\t\tUnlock flash in SPB mode by ESSPB command \r\n");
          return MxSpbUnlock(Mxic, ofs_s, ofs_e);
      }

      if (LockMode == ASP_PWD_MODE) {
          Mx_printf( "ERROR: please use 'PASSWORD UNLOCK SPB MODE' to unlock SPBs with password protection mode\n");
          return MXST_FAILURE;
      }
      return Status;
  }

   /*
    * Function:      MxAspIsLocked
    * Arguments:     Mxic,  pointer to an mxchip structure of nor flash device.
    *                Addr,  32 bit flash memory address of check area in advanced sector protection mode.
    *                Len,   number of bytes to check area in advanced sector protection mode.
    * Return Value:  MXST_FAILURE.
    *                  FLASH_IS_UNLOCKED.
    *                FLASH_IS_LOCKED.
    * Description:   This function is for checking if the block (or sector) is locked in advanced protection mode.
    */
   int MxAspIsLocked(MxChip *Mxic, u32 Addr, u64 Len)
   {
       int Status, LockMode;
       u32 ofs_s, ofs_e, ofs_s_org, ofs_e_org, wp64k_first, wp64k_last;

       LockMode = MxQryLockMode(Mxic);
       if (LockMode == MXST_FAILURE)
           return LockMode;

       if (LockMode == BP_MODE)
           return MxIsLocked(Mxic, Addr, Len);

       ofs_s_org = ofs_s = Addr;
       ofs_e_org = ofs_e = Addr + Len -1;
       wp64k_first = MX_WP64K_FIRST;
       wp64k_last = MX_WP64K_LAST(Mxic->ChipSz);
       Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 1);
       if (Status)
           return Status;

       if (ofs_s_org != ofs_s && ofs_s)
           ofs_s -= (ofs_s <= wp64k_first || ofs_s > wp64k_last) ? MX_4KB : MX_64KB;

       if (ofs_e_org != ofs_e && ofs_e)
           ofs_e += (ofs_e < wp64k_first || ofs_e >= wp64k_last) ? MX_4KB : MX_64KB;

       return MxSpbIsLocked(Mxic, ofs_s, ofs_e);
   }

/*
 * Function:      MxAspWpselLock
 * Arguments:      Mxic:  pointer to an mxchip structure of nor flash device.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for changing into ASP mode by setting the bit 7 in security register (WPSEL).
*/
int MxAspWpselLock(MxChip *Mxic)
{
    return MxWPSEL(Mxic);
}

/*
 * Function:      MxPwdAuth
 * Arguments:      Mxic,     pointer to an mxchip structure of nor flash device.
 *                   Password, data buffer to store the 64 bit password.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for inputing password to lock or unlock SPB protection.
 */
int MxPwdAuth(MxChip *Mxic, u8 *Password)
{
    return MxPASSULK(Mxic, Password);
}

/*
 * Function:      MxPwdLockUnlockSpb
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of locked or unlocked area in password protection mode.
 *                Len,   number of bytes to lock or unlock in password protection mode.
 *                Pwd,   data buffer to store the 64 bit password.
 *                IsLockSpb, flag to determine to perform lock or unlock SPB
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function will write protection a specified block (or sector) or cancel the block (or sector)
 *                write protection state in password protection mode.
 */
static int MxPwdLockUnlockSpb(MxChip *Mxic, u32 Addr, u64 Len, u8 *Pwd, u8 IsLockSpb)
{
    int Status, LockMode;
    u32 ofs_s, ofs_e;

    LockMode = MxQryLockMode(Mxic);
    if (LockMode == MXST_FAILURE)
        return LockMode;
    if ((LockMode == BP_MODE) || (LockMode == ASP_SOLID_MODE)) {
        Mx_printf("ERROR: please use 'SET PASSWORD MODE' to enter password protection mode\n");
        return MXST_FAILURE;
    }

    Status = MxPwdAuth(Mxic, Pwd);
    if (Status != MXST_SUCCESS)
        return Status;

    /* lock SPBs */
    ofs_s = Addr;
    ofs_e = Addr +Len -1;
    Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
    if (Status != MXST_SUCCESS)
        return Status;

    return IsLockSpb ? MxSpbLock(Mxic, ofs_s, ofs_e) : MxSpbUnlock(Mxic, ofs_s, ofs_e);
}

/*
 * Function:      MxAspLock
 * Arguments:      Mxic,  pointer to an mxchip structure of nor flash device.
 *                Addr,  32 bit flash memory address of locked area in password protection mode.
 *                Len,   number of bytes to lock in password protection mode.
 * Return Value:  MXST_SUCCESS.
 *                MXST_FAILURE.
 * Description:   This function is for writing protection a specified block (or sector) of flash memory in password protection mode.
 */
 int MxPwdLockSpb(MxChip *Mxic, u32 Addr, u64 Len)
{
    return MxPwdLockUnlockSpb(Mxic, Addr, Len, Mxic->Pwd, 1);
}

 /*
  * Function:      MxPwdUnlockSpb
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  32 bit flash memory address of unlocked area in password protection mode.
  *                Len,   number of bytes to unlock in password protection mode.
  * Return Value:  MXST_SUCCESS.
  *                MXST_FAILURE.
  * Description:   This function will cancel the block (or sector) write protection state in password protection mode.
  */
 int MxPwdUnlockSpb(MxChip *Mxic, u32 Addr, u64 Len)
{
    return MxPwdLockUnlockSpb(Mxic, Addr, Len, Mxic->Pwd, 0);
}

 /*
  * Function:      MxDpbLock
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  32 bit flash memory address of locked area in dynamic protection mode.
  *                Len,   number of bytes to lock in dynamic protection mode.
  * Return Value:  MXST_SUCCESS.
  *                MXST_FAILURE.
  * Description:   This function is for writing protection a specified block (or sector) of flash memory in dynamic protection mode.
  */
 int MxDpbLock(MxChip *Mxic, u32 Addr, u64 Len)
 {
     int Status;
     u32 ofs_s, ofs_e;

     ofs_s = Addr;
     ofs_e = Addr + Len -1;
     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
     if (Status != MXST_SUCCESS)
         return Status;
     return MxAspModeSet(Mxic, ofs_s, ofs_e, ASP_DPB_LOCK);
 }

 /*
  * Function:      MxDPBUnlockFlash
  * Arguments:       Mxic,  pointer to an mxchip structure of nor flash device.
  *                Addr,  32 bit flash memory address of unlocked area in dynamic protection mode.
  *                Len,   number of bytes to unlock in dynamic protection mode.
  * Return Value:  MXST_SUCCESS.
  *                MXST_FAILURE.
  * Description:   This function is for canceling the block (or sector) write protection state in dynamic protection mode.
  */
  int MxDpbUnlock(MxChip *Mxic, u32 Addr, u64 Len)
 {
     int Status;
     u32 ofs_s, ofs_e;

     ofs_s = Addr;
     ofs_e = Addr + Len -1;
     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
     if (Status != MXST_SUCCESS)
         return Status;
     return MxAspModeSet(Mxic, ofs_s, ofs_e, ASP_DPB_UNLOCK);
 }


  /*
   * Function:      MxIsFlashDPBLocked
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   *                Addr,  32 bit flash memory address of checking area in dynamic protection mode.
   *                Len,   number of bytes to check in dynamic protection mode.
   * Return Value:  MXST_FAILURE.
   *                 FLASH_IS_UNLOCKED.
   *                FLASH_IS_LOCKED.
   * Description:   This function is for checking if the block (or sector) is locked in dynamic protection mode.
   */
  int MxDpbIsLocked(MxChip *Mxic, u32 Addr, u64 Len)
 {
     int Status;
     u32 ofs_s_org, ofs_e_org, ofs_s, ofs_e, wp64k_first, wp64k_last;

     ofs_s_org = ofs_s = Addr;
     ofs_e_org = ofs_e = Addr + Len -1;
     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 1);
     if (Status != MXST_SUCCESS)
         return Status;
     wp64k_first = MX_WP64K_FIRST;
     wp64k_last = MX_WP64K_LAST(Mxic->ChipSz);

     if (ofs_s_org != ofs_s && ofs_s)
         ofs_s -= (ofs_s <= wp64k_first || ofs_s > wp64k_last) ? MX_4KB : MX_64KB;

     if (ofs_e_org != ofs_e && ofs_e)
         ofs_e += (ofs_e < wp64k_first || ofs_e >= wp64k_last) ? MX_4KB : MX_64KB;

     return MxAspIsLockedCheck(Mxic, ofs_s, ofs_e, ASP_DPB);
 }

  /*
    * Function:      MxSingleBlockLock
    * Arguments:     Mxic,  pointer to an mxchip structure of nor flash device.
    *                Addr,  32 bit flash memory address of locked area.
    *                Len,   number of bytes to lock.
    * Return Value:  MXST_SUCCESS.
    *                MXST_FAILURE.
    * Description:   This function is for writing protection a specified block (or sector) of flash memory in single block protection mode.
    */
  int MxSingleBlockLock(MxChip *Mxic, u32 Addr, u64 Len)
 {

     int Status;
     u32 ofs_s, ofs_e;
     u8 Wpsel;

     Status = MxRDSCUR(Mxic, &Wpsel);
     if (Status != MXST_SUCCESS)
         return Status;

     if (!(Wpsel & WPSEL_MASK))
         return MxLock(Mxic, Addr, Len);

     ofs_s = Addr;
     ofs_e = Addr + Len -1;
     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
     if (Status)
         return Status;
     return MxAspModeSet(Mxic, ofs_s, ofs_e, ASP_SBLK);
 }

  /*
   * Function:      MxSingleBlockUnlock
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   *                Addr,  32 bit flash memory address of unlocked area.
   *                Len,   number of bytes to unlock.
   * Return Value:  MXST_SUCCESS.
   *                MXST_FAILURE.
   * Description:   This function is for canceling the block (or sector)  write protection state in single block protection mode.
   */
  int MxSingleBlockUnlock(MxChip *Mxic, u32 Addr, u64 Len)
 {
     int Status;
     u32 ofs_s, ofs_e;
     u8 Wpsel;

     Status = MxRDSCUR(Mxic, &Wpsel);
     if (Status != MXST_SUCCESS)
         return Status;

     if (!(Wpsel & WPSEL_MASK))
         return MxLock(Mxic, Addr, Len);

     ofs_s = Addr;
     ofs_e = Addr + Len -1;
     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 0);
     if (Status)
         return Status;
     return MxAspModeSet(Mxic, ofs_s, ofs_e, ASP_SBULK);
 }

  /*
   * Function:      MxSingleBlockIsLocked
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   *                Addr,  32 bit flash memory address of checking area.
   *                Len,   number of bytes to check.
   * Return Value:  MXST_FAILURE.
   *                 FLASH_IS_UNLOCKED.
   *                FLASH_IS_LOCKED.
   * Description:   This function is for checking if the block (or sector) is locked in single block protection mode.
   */
  int MxSingleBlockIsLocked(MxChip *Mxic, u32 Addr, u64 Len)
 {
     int Status;
     u32 ofs_s_org, ofs_e_org, ofs_s, ofs_e, wp64k_first, wp64k_last;

     u8 Wpsel;

     Status = MxRDSCUR(Mxic, &Wpsel);
     if (Status != MXST_SUCCESS)
         return Status;

     if (!(Wpsel & WPSEL_MASK))
         return MxIsLocked(Mxic, Addr, Len);

     ofs_s_org = ofs_s = Addr;
     ofs_e_org = ofs_e = Addr + Len -1;
     Status = MxSetBoundary(Mxic, &ofs_s, &ofs_e, 1);
     if (Status)
         return Status;

     wp64k_first = MX_WP64K_FIRST;
     wp64k_last = MX_WP64K_LAST(Mxic->ChipSz);

     if (ofs_s_org != ofs_s && ofs_s)
         ofs_s -= (ofs_s <= wp64k_first || ofs_s > wp64k_last) ? MX_4KB : MX_64KB;

     if (ofs_e_org != ofs_e && ofs_e)
         ofs_e += (ofs_e < wp64k_first || ofs_e >= wp64k_last) ? MX_4KB : MX_64KB;

     return MxAspIsLockedCheck(Mxic, ofs_s, ofs_e, ASP_SB);
 }
#endif

  /*
   * Function:      MxChipReset
   * Arguments:        Mxic,  pointer to an mxchip structure of nor flash device.
   * Return Value:  MXST_SUCCESS.
   *                MXST_FAILURE.
   * Description:   This function is for reset the flash in software way.
   */
 int MxChipReset(MxChip *Mxic)
 {
    u8 Status;
    Status = MxRSTEN(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;
    Status = MxRST(Mxic);
    HAL_Delay(1); // Adjusted to suit ST HAL
    return Status;
 }
