/******************************************************************************
*
* @file mx_test.c
*
*
* Ver   Who   Date        Changes
* 1.00  RY      03/23/17    First release
* 2.00  RY      02/27/18    Change function names.
******************************************************************************/

#include "nor_ops.h"
#include "mx_test.h"

static u16 ErrorCnt = 0;
static u16 i = 0;
u32 cur_flash_addr = FLASH_INIT_ADDR;

int MxSimpleTest(MxChip *Mxic)
{
    int Status, n ;
    u8 Sr[2];
    MxSpi *Spi = Mxic->Priv;
    AppGrp * App = &Mxic->AppGrp;
    u8 WrData[TEST_SZ], RdData[TEST_SZ] = {0};
    u8 Id[30];

    for (n = 0; n < TEST_SZ; n++)
        WrData[n] = n % 0xFF;

    Mx_printf("  ------MxSimpleTest Start!------\r\n",0);

    if(Spi->CurMode & MODE_QPI)
        Status = MxQPIID(Mxic, 4, Id);
    else
        Status = MxRDID(Mxic, 4, Id);
    if (Status != MXST_SUCCESS)
        return 0;
    Mx_printf("  ID: %02X%02X%02X\r\n", Id[0], Id[1], Id[2]);

    Status = MxRDSR(Mxic, Sr);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("  Sr: %02X\r\n", Sr[0]);
    /*Sr[0] = SR_BP0;
    Sr[1] = SR_BP0;
    Status = MxWRSR(Mxic,Sr);
    if (Status != MXST_SUCCESS)
        return Status;
    Status = MxRDSR(Mxic, Sr);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("  Sr: %02X\r\n", Sr[0]);
    Sr[0] = 0;
    Sr[1] = 0;
    Status = MxWRSR(Mxic,Sr);
    if (Status != MXST_SUCCESS)
        return Status;
    Status = MxRDSR(Mxic, Sr);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("  Sr: %02X\r\n", Sr[0]);*/

    if (Mxic->SPICmdList[MX_ID_REG_CMDS] & MX_WRSCUR)
    {
        Status = MxRDSCUR(Mxic ,&Sr[0]);
        if (Status != MXST_SUCCESS)
            return Status;
        /*
         * WPSEL set, advanced protection mode
         */
        if( (Sr[0] & WPSEL_MASK) == WPSEL_MASK )
        {
            Mx_printf("\t\t WPSEL was set\r\n");
            #ifdef    BLOCK2_SERCURITY_OTP
                if (Mxic->SPICmdList[MX_MS_RST_SECU_SUSP] & MX_GBLK)
                {
                    Mx_printf("\t\t Unlock all DPB bit first\r\n");
                    /*
                     * DPB bit could be clear by GBULK command in advance protect method
                     */
                    Status = MxGBULK(Mxic);
                    if (Status != MXST_SUCCESS)
                        return Status;
                }
            #else
                Mx_printf("@Warning: User need enable BLOCK2_SERCURITY_OTP to unlock flash\r\n");
                return MXST_FAILURE;
            #endif
        }
    }

    /*
     * erase 64KB
     */
    Mx_printf("\tERASE START\r\n");
    Status = App->_Erase(Mxic, 0, 1);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("\tERASE DONE\r\n");
    /*
     * read TEST_SZ
     */
    Status = App->_Read(Mxic, 0, TEST_SZ, RdData);
    if (Status != MXST_SUCCESS)
        return Status;
    /*
     * check erase data
     */
    CompareValue( RdData, 0xFF, 0, TEST_SZ, 0);
    Mx_printf("\tERASE PASS\r\n");

    /*
     * write TEST_SZ
     */
    Status = App->_Write(Mxic, 0, TEST_SZ, WrData);
    if (Status != MXST_SUCCESS)
        return Status;
    Mx_printf("\tPROGRAM DONE\r\n");
    /*
     * read TEST_SZ
     */
    Status = App->_Read(Mxic, 0, TEST_SZ, RdData);
    if (Status != MXST_SUCCESS)
        return Status;
    /*
     * compare data
     */
    CompareData( RdData, WrData, 0, TEST_SZ, 0);

    Status = App->_Erase(Mxic, 0, 1);
    if (Status != MXST_SUCCESS)
        return Status;

    Mx_printf("\tMxSelfTest End!\r\n",0);
    return MXST_SUCCESS;
}

#ifdef BLOCK4_TESTBENCH

int MxComplicatedTest(MxChip *Mxic)
{
    int Status;
    u8  Sel;
    u32 SetMode;
    u32 SetFreq;
    MxSpi *Spi = Mxic->Priv;
    Mx_printf("\n\n== MX BSP DRIVER for TEST v0.1 ==\r\n");

    while (1) {
        Mx_printf("  1. SPI  \r\n");
        Mx_printf("  2. QPI  \r\n");
        Mx_printf("  3. SOPI \r\n");
        Mx_printf("  4. DOPI \r\n");
        Mx_printf("  q. Quit \r\n");
        Mx_printf("\nPlease enter your mode: \r\n");

        Sel = GetChar();
        Mx_printf("choose '%c'   \r\n", Sel);

        switch (Sel) {
        case '1':  SetMode = MODE_SPI;   break;
        case '2':  SetMode = MODE_QPI;   break;
        case '3':  SetMode = MODE_SOPI;  break;
        case '4':  SetMode = MODE_DOPI;  break;
        case 'q':  return MXST_SUCCESS;
        default:
            Mx_printf("wrong number, please choose again\r\n");
            continue;
        }
        break;
    }

    while (1) {
        Mx_printf("\n  1) 20 MHz   2) 50 MHz   3) 66 MHz   4) 100 MHz\r\n");
        Mx_printf("  5) 133 MHz  6) 150 MHz  7) 200 MHz  8) 250 MHz   9) 266 MHz\r\n");
        Mx_printf("  q. Quit\r\n");
        Mx_printf("\nPlease enter your frequency: \r\n");

        Sel = GetChar();
        Mx_printf("choose '%c'\r\n", Sel);

        switch (Sel) {
        case '1':  SetFreq = 20000000;    break;
        case '2':  SetFreq = 50000000;    break;
        case '3':  SetFreq = 66000000;    break;
        case '4':  SetFreq = 100000000;   break;
        case '5':  SetFreq = 133000000;   break;
        case '6':  SetFreq = 150000000;   break;
        case '7':  SetFreq = 200000000;   break;
        case '8':  SetFreq = 250000000;   break;
        case '9':  SetFreq = 266000000;   break;
        case 'q':  return MXST_SUCCESS;
        default:
            Mx_printf("wrong number, please choose again\r\n");
            continue;
        }
        break;
    }

    /* Change operation mode */
    if(Spi->CurMode != SetMode)
    {
        /*
         * Change mode is operated in SPI mode, the frequency should be not faster than 150MHz
         */
//        if(Mxic->CurFreq >= FREQ_SPI_MAX)
//        {
//            MxSetDeviceFreq(FREQ_SPI_MAX);
//        }
        Status = MxChangeMode(Mxic, SetMode,SELECT_4B);
        if (Status != MXST_SUCCESS)
            return Status;
    }

    /* Change frequency */
//    if ((SetMode != MODE_SOPI) && (SetMode != MODE_DOPI) && (SetFreq >= FREQ_SPI_MAX))
//    {
//        Mx_printf("\t@warning:the fast frequency is SPI mode is 150MHz,change to 150MHz\r\n");
//        SetFreq = FREQ_SPI_MAX;
//    }
    Mxic->CurFreq = SetFreq;
    MxSetDeviceFreq(Mxic->CurFreq);
    Status = MxCalibration(Mxic);
    if (Status != MXST_SUCCESS)
        return Status;

    while (1) {
        Mx_printf("\r\n  0. user API test\r\n");
        Mx_printf("  1. Performance test\r\n");
        Mx_printf("  2. Command set test\r\n");
        Mx_printf("  3. HardWare test\r\n");
        Mx_printf("  q. Quit\r\n");
        Mx_printf("\nPlease enter your test mode: \r\n");

        Sel = GetChar();
        Mx_printf("choose '%c'\r\n", Sel);

        switch (Sel) {
        case '0':
            Mx_printf("------------ User API test------------\r\n");
            Status = MxUserApiTest(Mxic);
            if (Status != MXST_SUCCESS)
                return Status;
            break;
        case '1':
            Mx_printf("------------ Performance test ------------\r\n");
            Status = MxPerformanceTest(Mxic);
            if (Status != MXST_SUCCESS)
                return Status;
            break;
        case '2':
            Mx_printf("------------ Command set test ------------\r\n");
            Status = MxCommandTest(Mxic);
            if (Status != MXST_SUCCESS)
                return Status;
            break;
        case '3':
            Mx_printf("------------ HardWare test(Not implemented) ------------\r\n");
            break;
        case 'q':
            break;
        default:
            Mx_printf("wrong number, please choose again\n\r\n");
            break;
        }
        break;
    }
    return MXST_SUCCESS;
}

/*
 * Address request function:
 * Command can request Page(256Byte),
 * Sector(4KB), Block(32KB, 64KB) address
 */
u32 MxRequestAddress( u8 request_size )
{
    /*
     * decoder request
     */
    switch( request_size ){
    case PAGE:
        cur_flash_addr = cur_flash_addr + PAGE_SZ;
        break;
    case SECTOR:
        cur_flash_addr = (cur_flash_addr + SECTOR4KB_SZ) & 0xFFFFF000;
        break;
    case BLOCK32K:
        cur_flash_addr = (cur_flash_addr + BLOCK32KB_SZ) & 0xFFFF8000;
        break;
    case BLOCK64K:
        cur_flash_addr = (cur_flash_addr + BLOCK64KB_SZ) & 0xFFFF0000;
        break;
    default:
        break;
    }

    return cur_flash_addr;
}

#endif
