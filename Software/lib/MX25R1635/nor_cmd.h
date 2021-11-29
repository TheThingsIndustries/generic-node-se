/******************************************************************************
*
* @file nor_cmd.h
*
*
* Ver   Who   Date        Changes
* 1.00  AZ      02/23/17    First release
*
******************************************************************************/

#ifndef NOR_CMD_H_
#define NOR_CMD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "spi.h"

struct _MxChip;

typedef struct {
    int (*_HardwareInit)(struct _MxChip *, u32 EffectiveAddr);
    int (*_Write)(struct _MxChip *, u32 Addr, u32 Cnt, u8 *Buf);
    int (*_Read)(struct _MxChip *, u32 Addr, u32 Cnt, u8 *Buf);
    int (*_Erase)(struct _MxChip *, u32 Addr, u32 Cnt);
    int (*_Lock)(struct _MxChip *, u32 Addr, u64 Len);
    int (*_Unlock)(struct _MxChip *, u32 Addr, u64 Len);
    int (*_IsLocked)(struct _MxChip *, u32 Addr, u64 Len);
} AppGrp;

typedef struct _MxChip{
    void *Priv;
    AppGrp AppGrp;
    u8 Id[SPI_NOR_FLASH_MAX_ID_LEN];
    u32 ChipSz;
//    u32 ErsSz;
    u32 PageSz;
    u32 BlockSz;
    u16 N_Blocks;
    u32 ChipSupMode;
    u32 ChipSpclFlag;
    RdDummy *RdDummy ;
    u32 *SPICmdList;
    u32 *QPICmdList;
    u32 *OPICmdList;
    u8 Pwd[8];
    u32 tW;
    u32 tDP;
    u32 tBP;
    u32 tPP;
    u32 tSE;
    u32 tBE32;
    u32 tBE;
    u32 tCE;
    u32 tWREAW;
    u32 CurFreq;

    u8 WriteBuffStart;
} MxChip;

#define ALL_BANK_READY        0
#define BANK0_BUSY            1
#define BANK1_BUSY            2
#define BANK2_BUSY            3
#define BANK3_BUSY            4

/*
 * define device special flags
 */
#define ADDR_4BYTE_ONLY            0x00000001
#define ADDR_3BYTE_ONLY            0x00000002
#define SUPPORT_WRSR_CR            0x00000004
#define RDPASS_ADDR                0x00000008
#define SUPPORT_CR_1BIT            0x00000010
#define SUPPORT_CR_2BIT            0x00000020
#define SUPPORT_CR_1BIT_BIT7    0x00000040
#define SUPPORT_CR_ODS            0x00000080
#define SCUR_NO_LDSO            0x00000100
#define SUPPORT_RWW                0x00000200

enum AddressMode{
     SELECT_3B = 0x00,
     SELECT_4B = 0x01,
     ADDR_MODE_AUTO_SEL = 0x03,
};

/* status register definition */
#define SR_WIP               0x01
#define SR_WEL               0x02
#define SR_QE                0x40    /* Quad-IO enable bit */
#define SR_BP0                0x04    /* Block protect 0 */
#define SR_BP1                0x08    /* Block protect 1 */
#define SR_BP2                0x10    /* Block protect 2 */
#define SR_BP3                0x20    /* Block protect 3 */
#define    SR_BP_BIT_OFFSET    2        /* Offset to Block protect 0 */
#define    SR_BP_BIT_MASK        (SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0)
#define SR_SRWD                0x80    /* SR write protect */

/* Configuration register definition */
#define CR_DC_BIT7       0x80
#define CR_DC_BIT6       0x40
#define CR_4BYTE_MASK    0x20
#define CR_PBE_MASK      0x10
#define CR_TB_MASK       0x08
#define CR_ODS_MASK      0x01

/* 2nd Configuration Register bits. */
#define CR2_OPI_EN_ADDR            0x00000000    /* Octa I/O enable address */
#define        CR2_OPI_MASK            3
#define     CR2_DOPI_EN                2        /* Octa DTR OPI enable */
#define     CR2_SOPI_EN                1        /* Octa STR OPI enable */
#define     CR2_SPI_EN                0        /* Octa SPI enable */
#define CR2_DQS_EN_ADDR            0x00000200    /* DQS enable address */
#define     CR2_SOPI_DQS_EN            0x02      /* STR DQS enable */
#define     CR2_DOPI_PRECYCLE_EN    0x01    /* DTR DQS pre-cycle enable */
#define CR2_DC_ADDR                0x00000300    /* Dummy cycle Address */
#define        CR2_DC_MASK                7        /* Dummy cycle mask */
#define CR2_CRC_PREAM_EB_ADDR    0x00000500    /* CRC and preamble enable */
#define     CR2_CRC_CHUNK_MASK        (0x3 << 5)
#define     CR2_CRC_CHUNK_16B        (0x0 << 5)
#define     CR2_CRC_CHUNK_32B        (0x1 << 5)
#define     CR2_CRC_CHUNK_64B        (0x2 << 5)
#define     CR2_CRC_CHUNK_128B        (0x3 << 5)
#define        CR2_CRC_EN                (0x1 << 4)
#define        CR2_PREAM_SEL_MASK        1
#define        CR2_PREAM_SEL_0            0
#define        CR2_PREAM_SEL_1            1

/* security register definition */
#define WPSEL_MASK            0x80
#define SCUR_EFB            0x40    /* erase failed bit   */
#define SCUR_PFB            0x20    /* program failed bit   */
#define SCUR_ESB            0x08    /* erase suspend bit   */
#define SCUR_PSB            0x04    /* program suspend bit */
#define SCUR_4BYTE_MASK         0x04    /* 4 byte bit */
#define SCUR_LDSO           0x02    /* indicate if security OTP is locked down */

/* lock register definition */
#define LR_SP_MASK            0x02
#define LR_PWDP_MASK        0x04
#define LR_SPBLB_MASK        0x40
#define LR_SPBLB_LOCK_EN       0xBF
#define LR_PWD_EN            0xFB
#define LR_SP_EN            0xFD

/* SPB lock register definition */
#define SPBLR_SPBLB_MASK    0x01

/* SPB register definition */
#define SPB_LOCKED            0xFF
#define SPB_UNLOCKED        0x00

/* DPB register definition */
#define DPB_LOCKED            0xFF
#define DPB_UNLOCKED        0x00

/* factory mode status register definition */
#define FM_EN                0xFF
#define FM_NOT_EN            0x00

/* extended address register definition */
#define EAR_BIT0_MASK        0x01

/* wrap depth definition for burst read*/
#define WRAP_DEPTH_8B        0x00
#define WRAP_DEPTH_16B        0x01
#define WRAP_DEPTH_32B        0x02
#define WRAP_DEPTH_64B       0x03
#define NO_WRAP_AROUND      0x10

#define MX_ID_REG_CMDS      0
#define MX_RD_CMDS          1
#define MX_PGM_ERS_CMDS     2
#define MX_MS_RST_SECU_SUSP 3

enum MX_CMD_ID_REG_CMDS {
    /* ID CMDs [11:00] */
    MX_RES        = 0x00000001, /* Read Electronic Signature */
    MX_REMS       = 0x00000002, /* Read Electronic Manufacturer ID & Device ID */
    MX_REMS2      = 0x00000004, /* Read Electronic Manufacturer ID & Device ID for 2xIO */
    MX_REMS4      = 0x00000008, /* Read Electronic Manufacturer ID & Device ID for 4xIO */
    MX_REMS4D     = 0x00000010, /* Read Electronic Manufacturer ID & Device ID for 4xIO-DDR */
    MX_QPIID      = 0x00000020,
    MX_RDID       = 0x00000040,

    /* REG CMDs [12:31] */
    MX_WRSR       = 0x00001000, /* Write Status Register */
    MX_WRSCUR     = 0x00002000, /* Write Security Register */
    MX_RDSCUR     = 0x00004000, /* Read Security Register */
    MX_RDCR       = 0x00008000, /* Read Configuration Register */
    MX_CLSR       = 0x00010000, /* Clear Status Register Fail Flags */
    MX_CR2        = 0x00020000, /* RDCR2, WRCR2: Read/Write Configuration Register 2 */
    MX_EAR        = 0x00040000, /* RDEAR, WREAR: Read/Write Extended Address Register */
    MX_FBR        = 0x00080000, /* RDFBR, WRFBR ESFBR: Read/Write/Erase Fast Boot Register */
    MX_LR         = 0x00100000, /* RDLR, WRLR: Read/Write Lock Register */
    MX_PASS       = 0x00200000, /* RDPASS, WRPASS PASSULK: ASP Password Protect */
    MX_SPB        = 0x00400000, /* RDSPB, WRSPB ESSPB: Read/Write/Erase Solid Protection Mode */
    MX_SPBLK      = 0x00800000, /* SPBLK, RDSPBLK: SPB Lock Bit */
    MX_DPB        = 0x01000000, /* RDDPB, WRDPB: Read/Write Dynamic  Protection Mode */
    MX_WRCR       = 0x02000000, /* Write Configuration Register */
    MX_RDFSR      = 0x04000000  /* Read Factory Register */
};
enum MX_CMD_RD_CMDS {
    /* READ CMDs [31:00] */
    MX_2READ      = 0x00000001, /* 1-2-2 FAST READ */
    MX_4READ      = 0x00000002, /* 1-4-4 FAST READ from bottom */
    MX_4READ_TOP  = 0x00000004, /* 1-4-4 FAST READ from top */
    MX_FASTREAD   = 0x00000008, /* 1-1-1 FAST READ */
    MX_DREAD      = 0x00000010, /* 1-1-2 FAST READ */
    MX_QREAD      = 0x00000020, /* 1-1-4 FAST READ */
    MX_FASTDTRD   = 0x00000040, /* 1-1-1 FAST READ DTR */
    MX_2DTRD      = 0x00000080, /* 1-2-2 FAST READ DTR */
    MX_4DTRD      = 0x00000100, /* 1-4-4 FAST READ DTR */
    MX_FRDTR4B    = 0x00000200, /* 1-1-1 FAST READ DTR 4-byte addressing */
    MX_2DTR4B     = 0x00000400, /* 1-2-2 FAST READ DTR 4-byte addressing */
    MX_4DTR4B     = 0x00000800, /* 1-4-4 FAST READ DTR 4-byte addressing */
    MX_W4READ     = 0x00001000, /* 1-4-4 FAST READ with fixed 4 dummy cycles */
    MX_CFIRD      = 0x00002000, /* READ CFI */
    MX_RDDMC      = 0x00004000, /* READ DMC */
    MX_RDSFDP     = 0x00008000, /* READ SFDP */
    MX_READ4B     = 0x00010000, /* 1-1-1 READ 4-byte addressing */
    MX_FASTREAD4B = 0x00020000, /* 1-1-1 FAST READ 4-byte addressing */
    MX_2READ4B    = 0x00040000, /* 1-2-2 FAST READ 4-byte addressing */
    MX_4READ4B    = 0x00080000, /* 1-4-4 FAST READ 4-byte addressing */
    MX_FREAD      = 0x00100000,
    MX_DREAD4B    = 0x00200000, /* 1-1-2 FAST READ 4-byte addressing */
    MX_QREAD4B    = 0x00400000, /* 1-1-4 QREAD 4-byte addressing */
    MX_8READ      = 0x00800000, /* 8-8-8 8READ 4-byte addressing */
    MX_8DTRD      = 0x01000000, /* 8D-8D-8D 8DTRD 4-byte addressing */
    MX_READ       = 0x02000000, /* 1-1-1  READ */
    MX_4B_RD      = (MX_READ4B | MX_FASTREAD4B | MX_2READ4B | MX_4READ4B | MX_DREAD4B | MX_QREAD4B)
};
enum MX_CMD_GM_ERS_CMDS {
#define MX_PGM_CMDS_MASK 0x0000FFFF
    /* PROGRAM CMDs [15:00] */
    MX_4PP       = 0x00000001, /* 1-4-4 Page PGM */
    MX_QPP       = 0x00000002, /* 1-1-4 Page QPGM */
    MX_CP        = 0x00000004, /* Chip PGM */
    MX_PP4B      = 0x00000008, /* 1-1-1 Page PGM 4-byte addressing */
    MX_4PP4B     = 0x00000010, /* 1-4-4 Page PGM 4-byte addressing */
    MX_PP        = 0x00000020, /* 1-1-1 Page PGM */
    MX_4B_PGM    = (MX_PP4B | MX_4PP4B),
#define MX_ERS_CMDS_MASK 0xFFFF0000
    /* ERASE CMDs [31:16] */
    MX_SE4B      = 0x00010000, /* Sector ERS 4-byte addressing */
    MX_BE32K     = 0x00020000, /* BLock ERS with 32Kbit */
    MX_BE32K4B   = 0x00040000, /* BLock ERS with 32KbitMX_BE32K4B 4-byte addressing */
    MX_BE        = 0x00080000, /* Block ERS */
    MX_BE4B      = 0x00100000, /* Block ERS 4-byte addressing */
    MX_CE        = 0x00200000, /* Chip ERS */
    MX_SE        = 0x00400000, /* Sector ERS 3-byte addressing */
    MX_4B_ERS    = (MX_SE4B |  MX_BE32K4B | MX_BE4B),
    MX_4B_PGMERS = (MX_4B_PGM | MX_4B_ERS)
};
enum MX_CMD_RST_SECU_SUSP {
    /* MODE SETTING CMDs [15:00]*/
    MX_DP        = 0x00000001, /* Deep Power Down Mode */
    MX_RDP         = 0x00000002, /* Release from Deep Power Down Mode */
    MX_SO        = 0x00000004, /* Enter/Exit Secured OTP */
    MX_SA        = 0x00000008, /* Enter/Exit Secured Area */
    MX_SRY       = 0x00000010, /* Enable/Disable SO to Output Ready/Busy */
    MX_EQIO      = 0x00000020, /* Enable QPI */
    MX_WPSEL     = 0x00000040, /* Enable block protect mode */
    MX_SBL       = 0x00000080, /* Set Burst Length */
    MX_ENPLM     = 0x00000100, /* Enter Parallel Mode, 8I/O */
    MX_EXPLM     = 0x00000200, /* Exit Parallel Mode, 8I/O */
    MX_4B        = 0x00000400, /* Enter/Exit 4-byte addressing mdoe */
    MX_HDE       = 0x00000800, /* Hold# Enable */
    MX_FMEN      = 0x00001000, /* Factory Mode Enable */

    /* Reset CMDs [19:16]*/
    MX_RST       = 0x00010000, /* RSTEN, RST */
    MX_RSTQIO    = 0x00020000, /* reset QIO */
    MX_NOP         = 0x00040000,

    /* Security CMDs [30:20]*/
    MX_SBLK      = 0x00100000, /* SBLK, SBULK, RDBLOCK */
    MX_GBLK      = 0x00200000, /* GBLK, GBULK */
    MX_KEY       = 0x00400000, /* KEY1, KEY2 */
    MX_BLOCKP    = 0x00800000, /* BLOCKP, RDBLOCK2, UNLOCK */
    MX_WRLB      = 0x01000000, /* Write Read-Lock Reg. bit2 */
    MX_PLOCK     = 0x02000000, /* PLOCK(Permanent Lock), RDPLOCK(Read Permanent Lock Status) */
    MX_RLCR      = 0x04000000, /* WRLCR(Write Read-Lock Reg.), RRLCR(Read Read-Lock Reg.) */
    MX_PLLK      = 0x08000000, /* PLLK(Permanent Lock Bit Lock Down), RDPLLK(Read Permanent Lock Bit Lock Down) */

    /* Suspend / Resume CMDs [31:31]*/
    MX_SUS_RES   = 0x80000000  /* Suspend/Resume Program or Erase */
};

/**********************�x        1.ID commands      �x      *************************/
#define MX_CMD_RDID                    0x9F        /* Read Identification */
#define MX_CMD_RES                    0xAB
#define MX_CMD_REMS                    0x90
#define MX_CMD_REMS2                0xEF
#define MX_CMD_REMS4                0xDF
#define MX_CMD_REMS4D                0xCF
#define MX_CMD_QPIID                0xAF

/**********************�x        2.Register commands      �x*************************/
#define MX_CMD_WRSR                    0x01        /* Write Status Register and Configuration Register */
#define MX_CMD_RDSR                    0x05        /* Status read command */
#define MX_CMD_WRSCUR                0x2F
#define MX_CMD_RDSCUR                0x2B
#define MX_CMD_RDCR                    0x15        /* Read Configuration Register */
#define MX_CMD_WRCR                    0x01        /* Read Configuration Register */
#define MX_CMD_CLSR                    0x30
#define MX_CMD_WRCR2                0x72        /* Write Configuration Register 2 */
#define MX_CMD_RDCR2                0x71        /* Read Configuration Register 2 */
#define MX_CMD_RDEAR                0xC8
#define MX_CMD_WREAR                0xC5
#define MX_CMD_RDFBR                0x16
#define MX_CMD_WRFBR                0x17
#define MX_CMD_ESFBR                0x18
#define MX_CMD_WRLR                    0x2C
#define MX_CMD_RDLR                  0x2D
#define MX_CMD_WRPASS                0x28
#define MX_CMD_RDPASS                0x27
#define MX_CMD_PASSULK                0x29
#define MX_CMD_RDSPB                0xE2
#define MX_CMD_WRSPB                0xE3
#define MX_CMD_ESSPB                0xE4
#define MX_CMD_SPBLK                0xA6
#define MX_CMD_RDSPBLK                0xA7
#define MX_CMD_WRDPB                0xE1
#define MX_CMD_RDDPB                0xE0
#define MX_CMD_RDFSR                0x44

/**********************�x        3.Read commands      �x    *************************/
#define MX_CMD_READ                    0x03
#define MX_CMD_FASTREAD                0x0B
#define MX_CMD_2READ                0xBB        /* Dual IO Fast Read */
#define MX_CMD_DREAD                0x3B        /* Dual Output Fast Read */
#define MX_CMD_4READ_BOTTOM            0xEB        /* Quad IO Fast Read */
#define MX_CMD_4READ_TOP            0xEA        /* Quad IO Fast Read */
#define MX_CMD_QREAD                0x6B        /* Quad Output Fast Read */
#define MX_CMD_FASTDTRD                0x0D
#define MX_CMD_2DTRD                0xBD
#define MX_CMD_4DTRD                0xED
#define MX_CMD_FASTDTRD4B            0x0E
#define MX_CMD_2DTRD4B                0xBE
#define MX_CMD_4DTRD4B                0xEE
#define MX_CMD_READ4B                0x13
#define MX_CMD_FASTREAD4B            0x0C
#define MX_CMD_2READ4B                0xBC        /* Dual IO Fast Read */
#define MX_CMD_DREAD4B                0x3C        /* Dual Output Fast Read */
#define MX_CMD_4READ4B                0xEC        /* Quad IO Fast Read */
#define MX_CMD_QREAD4B                0x6C        /* Quad Output Fast Read */
#define MX_CMD_8READ                0xEC        /* SOPI Read */
#define MX_CMD_8DTRD                0xEE        /* DOPI Read */
#define MX_CMD_RDSFDP                0x5A
#define MX_CMD_RDDMC                0x5B
#define MX_CMD_CFIRD                0x5C
#define MX_CMD_W4READ                0xE7
#define MX_CMD_FREAD                0xE6
#define MX_CMD_RDBUF                0x25

/**********************�x        4.Program commands      �x *************************/
#define    MX_CMD_WREN                    0x06        /* Write Enable command */
#define    MX_CMD_WRDI                    0x04
#define MX_CMD_PP                    0x02        /* Page Program command */
#define MX_CMD_4PP                    0x38        /* Quad Input Fast Program */
#define MX_CMD_QPP                    0x32
#define MX_CMD_PP4B                    0x12        /* Page Program command */
#define MX_CMD_4PP4B                0x3E        /* Quad Input Fast Program */
#define MX_CMD_CP                    0xAD
#define MX_CMD_WRBI                    0x22
#define MX_CMD_WRCT                    0x24
#define MX_CMD_WRCF                    0x31

/**********************�x        5.Erase commands      �x   *************************/

#define MX_CMD_SE                    0x20
#define MX_CMD_BE32K                0x52
#define MX_CMD_BE                    0xD8
#define MX_CMD_CE                    0xC7        /* Bulk Erase command */
#define MX_CMD_BE4B                    0xDC
#define MX_CMD_BE32K4B                0x5C
#define MX_CMD_SE4B                    0x21

/**********************�x        6.Mode setting commands    *************************/
#define MX_CMD_DP                    0xB9
#define MX_CMD_RDP                    0xAB
#define MX_CMD_ENSO                    0xB1
#define MX_CMD_EXSO                    0xC1
#define MX_CMD_ENSA                    0xB1
#define MX_CMD_EXSA                    0xC1
#define MX_CMD_ESRY                    0x70
#define MX_CMD_DSRY                    0x80
#define MX_CMD_EQPI                    0x35        /* Enable QPI Mode */
#define MX_CMD_WPSEL                0x68
#define MX_CMD_SBL                    0xC0
#define MX_CMD_EN4B                    0xB7
#define MX_CMD_EX4B                    0xE9
#define MX_CMD_FMEN                    0x41

/**********************�x        7.Reset commands      �x   *************************/

#define MX_CMD_RSTEN                0x66
#define MX_CMD_RST                    0x99
#define MX_CMD_RSTQPI                0xF5        /* Exit QPI Mode */

/**********************�x        8.Security commands      �x*************************/

#define MX_CMD_GBLK                    0x7E
#define MX_CMD_GBULK                0x98
#define MX_CMD_SBLK                    0x36
#define MX_CMD_SBULK                0x39
#define MX_CMD_RDBLOCK                0x3C
#define MX_CMD_RDBLOCK2                0xFB
#define MX_CMD_RDPLOCK                0x3F

/**********************�x        9.Suspend/Resume commands  *************************/

#define MX_CMD_PGMERS_SUSPEND        0xB0
#define MX_CMD_PGMERS_RESUME        0x30
#define MX_CMD_NOP                    0x00


/**********************�x        1.ID commands      �x      *************************/
int MxRDID(MxChip *Mxic, u32 ByteCount, u8 *Buf);
int MxRES(MxChip *Mxic, u8 *Buf);
int MxREMS(MxChip *Mxic,u32 Addr, u8 *Buf);
int MxREMS2(MxChip *Mxic,u32 Addr, u8 *Buf);
int MxREMS4(MxChip *Mxic,u32 Addr, u8 *Buf);
int MxREMS4D(MxChip *Mxic,u32 Addr, u8 *Buf);
int MxQPIID(MxChip *Mxic, u32 ByteCount, u8 *Buf);

/**********************�x        2.Register commands      �x*************************/
int MxWRSR(MxChip *Mxic, u8 *Buf);
int MxRDSR(MxChip *Mxic, u8 *Buf);
int MxRDSCUR(MxChip *Mxic, u8 *Buf);
int MxWRSCUR(MxChip *Mxic);
int MxRDCR(MxChip *Mxic, u8 *Buf);
int MxWRCR(MxChip *Mxic, u8 *Buf);
int MxCLSR(MxChip *Mxic);
int MxWRCR2(MxChip *Mxic , u32 Addr, u8 *Buf);
int MxRDCR2(MxChip *Mxic , u32 Addr, u8 *Buf);
int MxRDEAR(MxChip *Mxic, u8 *Buf);
int MxWREAR(MxChip *Mxic, u8 *Buf);
int MxWRFBR(MxChip *Mxic, u8 *Buf);
int MxRDFBR(MxChip *Mxic, u8 *Buf);
int MxESFBR(MxChip *Mxic);
int MxRDLR(MxChip *Mxic, u8 *Buf);
int MxWRLR(MxChip *Mxic, u8 *Buf);
int MxWRPASS(MxChip *Mxic, u8 *Buf);
int MxRDPASS(MxChip *Mxic, u8 *Buf);
int MxPASSULK(MxChip *Mxic, u8 *Buf);
int MxRDSPB(MxChip *Mxic, u32 Addr, u8 *Buf);
int MxWRSPB(MxChip *Mxi, u32 Addr);
int MxESSPB(MxChip *Mxic);
int MxSPBLK(MxChip *Mxic);
int MxRDSPBLK(MxChip *Mxic, u8 *Buf);
int MxRDDPB(MxChip *Mxic, u32 Addr, u8 *Buf);
int MxWRDPB(MxChip *Mxic, u32 Addr, u8 *Buf);
int MxRDFSR(MxChip *Mxic, u8 *Buf);

/**********************�x        3.Read commands      �x    *************************/
int MxREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxFASTREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxFASTDTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx2READ(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx2DTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxDREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx4READ(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx4READ_BOTTOM(MxChip *Mxic, u32 addr, u32 len, u8 *Buf);
int Mx4READ_TOP(MxChip *Mxic, u32 addr, u32 len, u8 *Buf);
int Mx4DTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxQREAD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx8READ(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx8DTRD(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxFASTREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxFASTDTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx2READ4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxDREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx4READ4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxQREAD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxFASTDTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx2DTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx4DTRD4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxRDSFDP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxRDBUF(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);

/**********************�x        4.Program commands      �x *************************/
int MxWREN(MxChip *Mxic);
int MxWRDI(MxChip *Mxic);
int MxPP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx4PP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx8PP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxQPP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxPP4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx4PP4B(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxCP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx8DTRPP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int Mx8PP(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxWRBI(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxWRCT(MxChip *Mxic, u32 Addr, u32 ByteCount, u8 *Buf);
int MxWRCF(MxChip *Mxic);

/**********************�x        5.Erase commands      �x   *************************/
int MxSE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);
int MxBE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);
int MxBE32K(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);
int MxCE(MxChip *Mxic);
int MxBE32K4B(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);
int MxSE4B(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);
int MxBE4B(MxChip *Mxic, u32 Addr, u32 ByteCount);
int MxBE(MxChip *Mxic, u32 Addr, u32 ByteCount);
int Mx8DTRBE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);
int Mx8BE(MxChip *Mxic, u32 Addr, u32 EraseSizeCount);

/**********************�x        6.Mode setting commands    *************************/
int MxDP(MxChip *Mxic);
int MxRDP(MxChip *Mxic);
int MxENSO(MxChip *Mxic);
int MxEXSO(MxChip *Mxic);
int MxENSA(MxChip *Mxic);
int MxEXSA(MxChip *Mxic);
int MxESRY(MxChip *Mxic);
int MxDSRY(MxChip *Mxic);
int MxEQPI(MxChip *Mxic);
int MxWPSEL(MxChip *Mxic);
int MxSBL(MxChip *Mxic, u8 *Buf);
int MxEN4B(MxChip *Mxic);
int MxEX4B(MxChip *Mxic);
int MxFMEN(MxChip *Mxic);

/**********************�x        7.Reset commands      �x   *************************/
int MxRSTEN(MxChip *Mxic);
int MxRST(MxChip *Mxic);
int MxRSTQPI(MxChip *Mxic);

/**********************�x        8.Security commands      �x*************************/
int MxGBLK(MxChip *Mxic);
int MxGBULK(MxChip *Mxic);
int MxSBLK(MxChip *Mxic,u32 Addr);
int MxSBULK(MxChip *Mxic,u32 Addr);
int MxRDBLOCK(MxChip *Mxic,u32 Addr, u8 *Buf);

/**********************�x        9.Suspend/Resume commands  *************************/
int MxPGMERS_SUSPEND(MxChip *Mxic);
int MxPGMERS_RESUME(MxChip *Mxic);
int MxNOP(MxChip *Mxic);

int MxWaitForFlashReady(MxChip *Mxic,u32 ExpectTime);
int MxIsFlashBusy(MxChip *Mxic);

#ifdef __cplusplus
}
#endif

#endif /* NOR_CMD_H_ */
