/*!
 * \file      LmHandler.c
 *
 * \brief     Implements the LoRaMac layer handling. 
 *            Provides the possibility to register applicative packages.
 *
 * \remark    Inspired by the examples provided on the en.i-cube_lrwan fork.
 *            MCD Application Team ( STMicroelectronics International )
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
/**
  ******************************************************************************
  *
  *          Portions COPYRIGHT 2020 STMicroelectronics
  *
  * @file    LmHandler.c
  * @author  MCD Application Team
  * @brief   LoRaMAC Layer handling definition
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "secure-element.h"

#include "LmHandler.h"
#include "Region.h"
#include "mw_log_conf.h"  /* needed for MW_LOG */
#include "lora_mac_version.h"
#include "Commissioning.h"
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
#else
#include "kms.h"
#include "kms_platf_objects_interface.h"
#endif

#include "NvmCtxMgmt.h"
#include "LmHandlerInfo.h"
#include "LmhpCompliance.h"
#include "LoRaMacTest.h"
#if (!defined (LORAWAN_DATA_DISTRIB_MGT) || (LORAWAN_DATA_DISTRIB_MGT == 0)) 
#else /*LORAWAN_DATA_DISTRIB_MGT == 1*/
#include "LmhpDataDistribution.h"
#endif /*LORAWAN_DATA_DISTRIB_MGT*/


/* Private typedef -----------------------------------------------------------*/
typedef enum PackageNotifyTypes_e
{
    PACKAGE_MCPS_CONFIRM,
    PACKAGE_MCPS_INDICATION,
    PACKAGE_MLME_CONFIRM,
}PackageNotifyTypes_t;

/*!
 * Commissioning parameters
 */
typedef struct CommissioningParams_s
{
    uint8_t DevEui[8];
    uint8_t JoinEui[8];
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    uint8_t GenAppKey[16];
    uint8_t AppKey[16];
    uint8_t NwkKey[16];
#endif
    uint32_t NetworkId;
    uint32_t DevAddr;
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    uint8_t FNwkSIntKey[16];
    uint8_t SNwkSIntKey[16];
    uint8_t NwkSEncKey[16];
    uint8_t AppSKey[16];
#endif
}CommissioningParams_t;

/* Private define ------------------------------------------------------------*/
/*!
 * Package application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/* Private macro -------------------------------------------------------------*/
/*!
 * Hex 8 split buffer
 */
#define HEX8(X)   X[0], X[1], X[2], X[3], X[4], X[5], X[6], X[7]

/*!
 * Hex 16 split buffer
 */
#define HEX16(X)  HEX8(X), X[8], X[9], X[10], X[11], X[12], X[13], X[14], X[15]

/*!
 * Format commissioning keys
 */
#define RAW_TO_INT8A(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) {0x##a,0x##b,0x##c,0x##d,\
0x##e,0x##f,0x##g,0x##h,0x##i,0x##j,0x##k,0x##l,0x##m,0x##n,0x##o,0x##p}

#define FORMAT_KEY(...) RAW_TO_INT8A(__VA_ARGS__)

/* Private function prototypes -----------------------------------------------*/
/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *                             containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm );

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication );

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] MlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm );

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void MlmeIndication( MlmeIndication_t *mlmeIndication );

/*!
 * Requests network server time update
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SET if joined else \ref LORAMAC_HANDLER_RESET
 */
static LmHandlerErrorStatus_t LmHandlerDeviceTimeReq( void );

#if ( LORAMAC_CLASSB_ENABLED == 1 )
/*!
 * Starts the beacon search
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SET if joined else \ref LORAMAC_HANDLER_RESET
 */
static LmHandlerErrorStatus_t LmHandlerBeaconReq( void );

/*!
 * Informs the server on the ping-slot periodicity to use
 *
 * \param [IN] periodicity Is equal to 2^periodicity seconds.
 *                         Example: 2^3 = 8 seconds. The end-device will open an Rx slot every 8 seconds.
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SUCCESS if request has been
 *                processed else \ref LORAMAC_HANDLER_ERROR
 */
static LmHandlerErrorStatus_t LmHandlerPingSlotReq( uint8_t periodicity );
#endif

/*!
 * Notifies the package to process the LoRaMac callbacks.
 *
 * \param [IN] notifyType MAC notification type [PACKAGE_MCPS_CONFIRM,
 *                                               PACKAGE_MCPS_INDICATION,
 *                                               PACKAGE_MLME_CONFIRM,
 *                                               PACKAGE_MLME_INDICATION]
 * \param[IN] params      Notification parameters. The params type can be
 *                        [McpsConfirm_t, McpsIndication_t, MlmeConfirm_t, MlmeIndication_t]
 */
static void LmHandlerPackagesNotify( PackageNotifyTypes_t notifyType, void *params );

/*!
 * Check whether the Device is joined to the network
 *
 * \param [IN] none
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SET if joined else \ref LORAMAC_HANDLER_RESET
 */
static LmHandlerFlagStatus_t LmHandlerJoinStatus( void );

static bool LmHandlerPackageIsInitialized( uint8_t id );

/*!
 * \brief Displays end-device class update
 *
 * \param [IN] deviceClass Current end-device class
 */
static void DisplayClassUpdate( DeviceClass_t deviceClass );

#if ( LORAMAC_CLASSB_ENABLED == 1 )
/*!
 * \brief Displays beacon status update
 *
 * \param [IN] params Beacon parameters
 */
static void DisplayBeaconUpdate( LoRaMAcHandlerBeaconParams_t* params );
#endif

/* Private variables ---------------------------------------------------------*/
static CommissioningParams_t CommissioningParams = 
{
  .DevEui = LORAWAN_DEVICE_EUI,
  .JoinEui = LORAWAN_JOIN_EUI,
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  .GenAppKey = FORMAT_KEY(LORAWAN_GEN_APP_KEY),
  .AppKey = FORMAT_KEY(LORAWAN_APP_KEY),
  .NwkKey = FORMAT_KEY(LORAWAN_NWK_KEY),
#endif
  .NetworkId = LORAWAN_NETWORK_ID,
  .DevAddr = LORAWAN_DEVICE_ADDRESS,
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  .FNwkSIntKey = FORMAT_KEY(LORAWAN_F_NWK_S_INT_KEY),
  .SNwkSIntKey = FORMAT_KEY(LORAWAN_S_NWK_S_INT_KEY),
  .NwkSEncKey = FORMAT_KEY(LORAWAN_NWK_S_ENC_KEY),
  .AppSKey = FORMAT_KEY(LORAWAN_APP_S_KEY),
#endif
};

/*!
 * LoRaWAN compliance tests handler parameters
 */
static LmhpComplianceParams_t LmhpComplianceParams =
{
    .AdrEnabled =       LORAMAC_HANDLER_ADR_ON,
    .DutyCycleEnabled = false,
    .StopPeripherals =  NULL,
    .StartPeripherals = NULL,
};

static LmhPackage_t *LmHandlerPackages[PKG_MAX_NUMBER];

/*!
 * Upper layer LoRaMac parameters
 */
static LmHandlerParams_t LmHandlerParams;

/*!
 * Upper layer callbacks
 */
static LmHandlerCallbacks_t LmHandlerCallbacks;

/*!
 * Used to notify LmHandler of LoRaMac events
 */
static LoRaMacPrimitives_t LoRaMacPrimitives;

/*!
 * LoRaMac callbacks
 */
static LoRaMacCallback_t LoRaMacCallbacks;

static LmHandlerJoinParams_t JoinParams =
{
  .Datarate = DR_0,
  .Status = LORAMAC_HANDLER_ERROR,
  .Mode = ACTIVATION_TYPE_NONE
};

static LmHandlerTxParams_t TxParams =
{
  .MsgType = LORAMAC_HANDLER_UNCONFIRMED_MSG,
  .AckReceived = 0,
  .Datarate = DR_0,
  .UplinkCounter = 0,
  .AppData =
  {
    .Port = 0,
    .BufferSize = 0,
    .Buffer = NULL
  },
  .TxPower = TX_POWER_0,
  .Channel = 0
};

static LmHandlerRxParams_t RxParams =
{
  .Rssi = 0,
  .Snr = 0,
  .DownlinkCounter = 0,
  .RxSlot = -1
};

#if ( LORAMAC_CLASSB_ENABLED == 1 )
static LoRaMAcHandlerBeaconParams_t BeaconParams =
{
  .State = LORAMAC_HANDLER_BEACON_ACQUIRING,
  .Info =
  {
    .Time = { .Seconds = 0, .SubSeconds = 0 },
    .Frequency = 0,
    .Datarate = 0,
    .Rssi = 0,
    .Snr = 0,
    .GwSpecific =
    {
      .InfoDesc = 0,
      .Info = { 0 }
    }
  }
};
#endif

/*!
 * Package Application buffer
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * Package application data structure
 */
static LmHandlerAppData_t AppData = { 0, 0, AppDataBuffer };

#if ( LORAMAC_CLASSB_ENABLED == 1 )
/*!
 * Indicates if a switch to Class B operation is pending or not.
 */
static bool IsClassBSwitchPending = false;
#endif

static bool CtxRestoreDone = false;

/* Exported functions ---------------------------------------------------------*/
LmHandlerErrorStatus_t LmHandlerInit( LmHandlerCallbacks_t *handlerCallbacks )
{
  LmHandlerInfo_Init();
  
  UTIL_MEM_cpy_8((void *)&LmHandlerCallbacks, (const void *)handlerCallbacks, sizeof(LmHandlerCallbacks_t));
  
  MW_LOG(TS_OFF, VLEVEL_M, "MAC_VERSION= V%X.%X.%X_rc%X\r\n", (uint8_t)(__LORA_MAC_VERSION >> __LORA_MAC_MAIN_SHIFT), (uint8_t)(__LORA_MAC_VERSION >> __LORA_MAC_SUB1_SHIFT), (uint8_t)(__LORA_MAC_VERSION >> __LORA_MAC_SUB2_SHIFT),  (uint8_t)(__LORA_MAC_VERSION >> __LORA_MAC_VERSION_RC));

  LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
  LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
  LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
  LoRaMacPrimitives.MacMlmeIndication = MlmeIndication;
  LoRaMacCallbacks.GetBatteryLevel = LmHandlerCallbacks.GetBatteryLevel;
  LoRaMacCallbacks.GetTemperatureLevel = LmHandlerCallbacks.GetTemperature;
  LoRaMacCallbacks.NvmContextChange = NvmCtxMgmtEvent;
  LoRaMacCallbacks.MacProcessNotify = LmHandlerCallbacks.OnMacProcess;
  
  /*The LoRa-Alliance Compliance protocol package should always be initialized and activated.*/
  if ( LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams ) != LORAMAC_HANDLER_SUCCESS )
  {
    return LORAMAC_HANDLER_ERROR;
  }
#if (!defined (LORAWAN_DATA_DISTRIB_MGT) || (LORAWAN_DATA_DISTRIB_MGT == 0))
#else /*LORAWAN_DATA_DISTRIB_MGT == 1*/
  if ( LmhpDataDistributionInit() != LORAMAC_HANDLER_SUCCESS )
  {
    return LORAMAC_HANDLER_ERROR;
  }
#endif /*LORAWAN_DATA_DISTRIB_MGT*/
  return LORAMAC_HANDLER_SUCCESS;
}

LmHandlerErrorStatus_t LmHandlerConfigure( LmHandlerParams_t *handlerParams )
{
  MibRequestConfirm_t mibReq;
  LmHandlerInfo_t *lmHandlerInfo;

  UTIL_MEM_cpy_8((void *)&LmHandlerParams, (const void *)handlerParams, sizeof(LmHandlerParams_t));

#if ( LORAMAC_CLASSB_ENABLED == 1 ) 
  IsClassBSwitchPending = false;
#endif
  
  lmHandlerInfo = LmHandlerInfo_GetPtr();
  
  if( 0U != ((1<<(LmHandlerParams.ActiveRegion)) & (lmHandlerInfo->Region)) )
  {
    if( LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LmHandlerParams.ActiveRegion ) != LORAMAC_STATUS_OK )
    {
      return LORAMAC_HANDLER_ERROR;
    }
  }
  else
  {
    MW_LOG(TS_ON, VLEVEL_ALWAYS,"error: Region is not defined in the MW: set lorawan_conf.h accordingly\r\n");
    while(1) {}   /* error: Region is not defined in the MW */
  }
  
  // Try to restore from NVM and query the mac if possible.
  if( NvmCtxMgmtRestore( ) == NVMCTXMGMT_STATUS_SUCCESS )
  {
    CtxRestoreDone = true;
  }
  else
  {
    CtxRestoreDone = false;
    
#if ( STATIC_DEVICE_EUI != 1 )
    LmHandlerCallbacks.GetUniqueId(CommissioningParams.DevEui);
#endif

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    mibReq.Type = MIB_APP_KEY;
    mibReq.Param.AppKey = CommissioningParams.AppKey;
    LoRaMacMibSetRequestConfirm( &mibReq );
    
    mibReq.Type = MIB_NWK_KEY;
    mibReq.Param.NwkKey = CommissioningParams.NwkKey;
    LoRaMacMibSetRequestConfirm( &mibReq );
    
    mibReq.Type = MIB_GEN_APP_KEY;
    mibReq.Param.GenAppKey = CommissioningParams.GenAppKey;
    LoRaMacMibSetRequestConfirm( &mibReq );
#else /* LORAWAN_KMS == 1 */
    SecureElementSetObjHandler(APP_KEY, KMS_APP_KEY_OBJECT_HANDLE);
    SecureElementSetObjHandler(NWK_KEY, KMS_NWK_KEY_OBJECT_HANDLE);
    SecureElementSetObjHandler(GEN_APP_KEY, KMS_GEN_APP_KEY_OBJECT_HANDLE);
#if ( LORAMAC_CLASSB_ENABLED == 1 )
    SecureElementSetObjHandler(SLOT_RAND_ZERO_KEY, KMS_ZERO_KEY_OBJECT_HANDLE);
#endif /* LORAMAC_CLASSB_ENABLED */
#endif   
    mibReq.Type = MIB_DEV_EUI;
    mibReq.Param.DevEui = CommissioningParams.DevEui;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_JOIN_EUI;
    mibReq.Param.JoinEui = CommissioningParams.JoinEui;
    LoRaMacMibSetRequestConfirm( &mibReq );
  }
  MW_LOG(TS_OFF, VLEVEL_M, "###### DevEui:    %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n", HEX8(CommissioningParams.DevEui));
  MW_LOG(TS_OFF, VLEVEL_M, "###### AppEui:    %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n", HEX8(CommissioningParams.JoinEui));
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  MW_LOG(TS_OFF, VLEVEL_M, "###### AppKey:    %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", HEX16(CommissioningParams.NwkKey));
  MW_LOG(TS_OFF, VLEVEL_M, "###### GenAppKey: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", HEX16(CommissioningParams.GenAppKey));
#else
  MW_LOG(TS_OFF, VLEVEL_L, "###### KMS ENABLED \r\n");
#endif

  mibReq.Type = MIB_PUBLIC_NETWORK;
  mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_ADR;
  mibReq.Param.AdrEnable = LmHandlerParams.AdrEnable;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
  mibReq.Param.SystemMaxRxError = 20;
  LoRaMacMibSetRequestConfirm( &mibReq );

  GetPhyParams_t getPhy;
  PhyParam_t phyParam;
  getPhy.Attribute = PHY_DUTY_CYCLE;
  phyParam = RegionGetPhyParam( LmHandlerParams.ActiveRegion, &getPhy );
  LmHandlerParams.DutyCycleEnabled = ( bool ) phyParam.Value;
  
  // override previous value if reconfigure new region
  LoRaMacTestSetDutyCycleOn(LmHandlerParams.DutyCycleEnabled);
  
  return LORAMAC_HANDLER_SUCCESS;
}

/*!
 * Join a LoRa Network in classA
 *
 * \Note if the device is ABP, this is a pass through function
 *
 * \param [IN] isOtaa Indicates which activation mode must be used
 */
void LmHandlerJoin( ActivationType_t mode )
{
  MibRequestConfirm_t mibReq;
  
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
#else /* LORAWAN_KMS == 1 */
#if (OVER_THE_AIR_ACTIVATION == 0)
  if( mode == ACTIVATION_TYPE_OTAA )
  {
    MW_LOG(TS_OFF, VLEVEL_M, "ERROR: OTAA mode not implemented\r\n");
    while(1);
  }
#endif /* OVER_THE_AIR_ACTIVATION */
#if (ACTIVATION_BY_PERSONALISATION == 0)
  if( mode == ACTIVATION_TYPE_ABP )
  {
    MW_LOG(TS_OFF, VLEVEL_M, "ERROR: ABP mode not implemented\r\n");
    while(1);
  }
#endif /* ACTIVATION_BY_PERSONALISATION */
#endif /* LORAWAN_KMS */
  SecureElementDeleteDerivedKeys(NULL);

  if( mode == ACTIVATION_TYPE_OTAA )
  {
    MlmeReq_t mlmeReq;
    JoinParams.Mode = ACTIVATION_TYPE_OTAA;

    LoRaMacStart( );
    
    mlmeReq.Type = MLME_JOIN;
    mlmeReq.Req.Join.Datarate = LmHandlerParams.TxDatarate;
    LoRaMacMlmeRequest( &mlmeReq );

    // Starts the OTAA join procedure
    TimerTime_t nextTxIn = 0;
    LoRaMacQueryNextTxDelay( TxParams.Datarate, &nextTxIn );
  }
  else
  {
    JoinParams.Mode = ACTIVATION_TYPE_ABP;
    JoinParams.Status = LORAMAC_HANDLER_SUCCESS;

    if (CtxRestoreDone == false)
    {
#if ( STATIC_DEVICE_ADDRESS != 1 )
      // Choose a "pseudo-random" device address
      CommissioningParams.DevAddr = LmHandlerCallbacks.GetRandomSeed( );
#endif

      mibReq.Type = MIB_NET_ID;
      mibReq.Param.NetID = CommissioningParams.NetworkId;
      LoRaMacMibSetRequestConfirm( &mibReq );
      
      mibReq.Type = MIB_DEV_ADDR;
      mibReq.Param.DevAddr = CommissioningParams.DevAddr;
      LoRaMacMibSetRequestConfirm( &mibReq );
      MW_LOG(TS_OFF, VLEVEL_M, "###### DevAddr:   %08X\r\n", CommissioningParams.DevAddr);
          
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
      mibReq.Type = MIB_F_NWK_S_INT_KEY;
      mibReq.Param.FNwkSIntKey = CommissioningParams.FNwkSIntKey;
      LoRaMacMibSetRequestConfirm( &mibReq );
      MW_LOG(TS_OFF, VLEVEL_M, "###### NwkSKey:   %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
             HEX16(CommissioningParams.FNwkSIntKey));

      mibReq.Type = MIB_S_NWK_S_INT_KEY;
      mibReq.Param.SNwkSIntKey = CommissioningParams.SNwkSIntKey;
      LoRaMacMibSetRequestConfirm( &mibReq );
      
      mibReq.Type = MIB_NWK_S_ENC_KEY;
      mibReq.Param.NwkSEncKey = CommissioningParams.NwkSEncKey;
      LoRaMacMibSetRequestConfirm( &mibReq );
      
      mibReq.Type = MIB_APP_S_KEY;
      mibReq.Param.AppSKey = CommissioningParams.AppSKey;
      LoRaMacMibSetRequestConfirm( &mibReq );
      MW_LOG(TS_OFF, VLEVEL_M, "###### AppSKey:   %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
             HEX16(CommissioningParams.AppSKey));
#else /* LORAWAN_KMS == 1 */
    SecureElementSetObjHandler(F_NWK_S_INT_KEY, KMS_F_NWK_S_INT_KEY_OBJECT_HANDLE);
    SecureElementSetObjHandler(S_NWK_S_INT_KEY, KMS_S_NWK_S_INT_KEY_OBJECT_HANDLE);
    SecureElementSetObjHandler(NWK_S_ENC_KEY, KMS_NWK_S_ENC_KEY_OBJECT_HANDLE);
    SecureElementSetObjHandler(APP_S_KEY, KMS_APP_S_KEY_OBJECT_HANDLE);
#endif  /* LORAWAN_KMS */
    }
    
    LoRaMacStart( );
    mibReq.Type = MIB_NETWORK_ACTIVATION;
    mibReq.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
    LoRaMacMibSetRequestConfirm( &mibReq );
    
    LmHandlerCallbacks.OnJoinRequest( &JoinParams );
    LmHandlerRequestClass( LmHandlerParams.DefaultClass );
  }
}

/*!
 * Stop a LoRa Network connection
 *
 * \retval none
 */
void LmHandlerStop( void )
{
  LoRaMacStop( );
}

bool LmHandlerIsBusy( void )
{
  if( LoRaMacIsBusy( ) == true )
  {
    return true;
  }
  
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    // The network isn't yet joined, try again later.
    LmHandlerJoin( JoinParams.Mode );
    return true;
  }
  
  if( LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->IsRunning( ) == true )
  {
    return true;
  }
  return false;
}

LmHandlerErrorStatus_t LmHandlerSend( LmHandlerAppData_t *appData, LmHandlerMsgTypes_t isTxConfirmed, TimerTime_t *nextTxIn, bool allowDelayedTx )
{
  LoRaMacStatus_t status;
  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  
  if( LoRaMacIsBusy( ) == true )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    // The network isn't yet joined, try again later.
    LmHandlerJoin( JoinParams.Mode );
    return LORAMAC_HANDLER_ERROR;
  }

  if( ( LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->IsRunning( ) == true ) && ( appData->Port != LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->Port ) && ( appData->Port != 0 ) )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mcpsReq.Req.Unconfirmed.Datarate = LmHandlerParams.TxDatarate;
  if( LoRaMacQueryTxPossible( appData->BufferSize, &txInfo ) != LORAMAC_STATUS_OK )
  {
    // Send empty frame in order to flush MAC commands
    TxParams.MsgType = LORAMAC_HANDLER_UNCONFIRMED_MSG;
    mcpsReq.Type = MCPS_UNCONFIRMED;
    mcpsReq.Req.Unconfirmed.fBuffer = NULL;
    mcpsReq.Req.Unconfirmed.fBufferSize = 0;
  }
  else
  {
    TxParams.MsgType = isTxConfirmed;
    mcpsReq.Req.Unconfirmed.fPort = appData->Port;
    mcpsReq.Req.Unconfirmed.fBufferSize = appData->BufferSize;
    mcpsReq.Req.Unconfirmed.fBuffer = appData->Buffer;
    if( isTxConfirmed == LORAMAC_HANDLER_UNCONFIRMED_MSG )
    {
      mcpsReq.Type = MCPS_UNCONFIRMED;
    }
    else
    {
      mcpsReq.Type = MCPS_CONFIRMED;
      mcpsReq.Req.Confirmed.NbTrials = 8;
    }
  }
  
  TxParams.AppData = *appData;
  TxParams.Datarate = LmHandlerParams.TxDatarate;
  
  if (nextTxIn != NULL)
  {
    LoRaMacQueryNextTxDelay( TxParams.Datarate, nextTxIn );
  }
  status = LoRaMacMcpsRequest( &mcpsReq, allowDelayedTx );
  
  if( status == LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_SUCCESS;
  }
  else
  {
    return LORAMAC_HANDLER_ERROR;
  }
}

LmHandlerErrorStatus_t LmHandlerRequestClass( DeviceClass_t newClass )
{
  MibRequestConfirm_t mibReq;
  DeviceClass_t currentClass;
  LmHandlerErrorStatus_t errorStatus = LORAMAC_HANDLER_SUCCESS;
  
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    return LORAMAC_HANDLER_NO_NETWORK_JOINED;
  }
    
  mibReq.Type = MIB_DEVICE_CLASS;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  currentClass = mibReq.Param.Class;
  
  // Attempt to switch only if class update
  if( currentClass != newClass )
  {
    switch( newClass )
    {
    case CLASS_A:
      {
        if( currentClass != CLASS_A )
        {
          mibReq.Param.Class = CLASS_A;
          if( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
          {
            // Switch is instantaneous
            DisplayClassUpdate(CLASS_A);
          }
          else
          {
            errorStatus = LORAMAC_HANDLER_ERROR;
          }
        }
      }
      break;
    case CLASS_B:
      {
#if ( LORAMAC_CLASSB_ENABLED == 1 )
        if( currentClass != CLASS_A )
        {
          errorStatus = LORAMAC_HANDLER_ERROR;
        }
        else
        {
          // Beacon must first be acquired
          errorStatus = LmHandlerDeviceTimeReq( );
          IsClassBSwitchPending = true;
        }
#else
        errorStatus = LORAMAC_HANDLER_ERROR;
#endif
      }
      break;
    case CLASS_C:
      {
        if( currentClass != CLASS_A )
        {
          errorStatus = LORAMAC_HANDLER_ERROR;
        }
        else
        {
          // Switch is instantaneous
          mibReq.Param.Class = CLASS_C;
          if( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
          {
            DisplayClassUpdate(CLASS_C);
          }
          else
          {
            errorStatus = LORAMAC_HANDLER_ERROR;
          }
        }
      }
      break;
    default:
      break;
    }
  }
  return errorStatus;
}

LmHandlerErrorStatus_t LmHandlerPackageRegister( uint8_t id, void *params )
{
  LmhPackage_t *package = NULL;
  switch( id )
  {
  case PACKAGE_ID_COMPLIANCE:
    {
      package = LmphCompliancePackageFactory( );
      break;
    }
  default:
#if (!defined (LORAWAN_DATA_DISTRIB_MGT) || (LORAWAN_DATA_DISTRIB_MGT == 0))
#else /*LORAWAN_DATA_DISTRIB_MGT == 1*/
      LmhpDataDistributionPackageRegister(id, &package);
#endif /*LORAWAN_DATA_DISTRIB_MGT*/
      break;
  }

  if( package != NULL )
  {
    LmHandlerPackages[id] = package;
    LmHandlerPackages[id]->OnJoinRequest = LmHandlerJoin;
    LmHandlerPackages[id]->OnSendRequest = LmHandlerSend;
    LmHandlerPackages[id]->OnDeviceTimeRequest = LmHandlerDeviceTimeReq;
    LmHandlerPackages[id]->Init( params, AppData.Buffer, LORAWAN_APP_DATA_BUFFER_MAX_SIZE );
    
    return LORAMAC_HANDLER_SUCCESS;
  }
  else
  {
    return LORAMAC_HANDLER_ERROR;
  }
}

void LmHandlerPackagesProcess( void )
{
  // Call at first the LoRaMAC process before to run all package process features
  LoRaMacProcess();

  for( int8_t i = 0; i < PKG_MAX_NUMBER; i++ )
  {
    if(( LmHandlerPackages[i] != NULL ) &&
       ( LmHandlerPackages[i]->Process != NULL ) &&
       ( LmHandlerPackageIsInitialized( i ) != false ) )
    {
      LmHandlerPackages[i]->Process( );
    }
  }
  
  NvmCtxMgmtStore( );
}

int32_t LmHandlerGetCurrentClass( DeviceClass_t *deviceClass )
{
  MibRequestConfirm_t mibReq;
  if (deviceClass == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  mibReq.Type = MIB_DEVICE_CLASS;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  *deviceClass = mibReq.Param.Class;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetDevEUI( uint8_t *devEUI )
{
  if (devEUI == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  UTIL_MEM_cpy_8( devEUI, CommissioningParams.DevEui, 8 );
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetDevEUI( uint8_t *devEUI )
{
#if( STATIC_DEVICE_EUI != 1 )
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    UTIL_MEM_cpy_8( CommissioningParams.DevEui, devEUI, 8 );
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change Keys in running state
    return LORAMAC_HANDLER_ERROR;
  }
#else
 return LORAMAC_HANDLER_ERROR; 
#endif
}

int32_t LmHandlerGetAppEUI( uint8_t *appEUI )
{
  if (appEUI == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  UTIL_MEM_cpy_8( appEUI, CommissioningParams.JoinEui, 8 );
 
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetAppEUI( uint8_t *appEUI )
{
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    UTIL_MEM_cpy_8( CommissioningParams.JoinEui, appEUI, 8 );
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change Keys in running state
    return LORAMAC_HANDLER_ERROR;
  }
}

int32_t LmHandlerGetAppKey( uint8_t *appKey )
{
  if (appKey == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  UTIL_MEM_cpy_8( appKey, CommissioningParams.NwkKey, 16 );  
  return LORAMAC_HANDLER_SUCCESS;
#else /* LORAWAN_KMS == 1 */
 /*can only be done if the key attribute is EXTRACTABLE */
 /* in the project ST provide for Lora it will never be EXTRACTABLE */
  return LORAMAC_HANDLER_ERROR;
#endif  /* KMS */
}

int32_t LmHandlerSetAppKey( uint8_t *appKey )
{
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    UTIL_MEM_cpy_8( CommissioningParams.NwkKey, appKey, 16 );
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change Keys in running state
    return LORAMAC_HANDLER_ERROR;
  }
#else /* LORAWAN_KMS == 1 */
  /* this feature is currently not supported by KMS */
  /* Ticket 80993: The function CreateObject will be used when available on Kms */
  /* It will create an key in NVM and remap the init table on that handler */
  return LORAMAC_HANDLER_ERROR;
#endif  /* KMS */
}

int32_t LmHandlerGetNetworkID( uint32_t *networkId )
{
  if (networkId == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *networkId = CommissioningParams.NetworkId;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetNetworkID( uint32_t networkId )
{
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    CommissioningParams.NetworkId = networkId;
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change NetworkID in running state
    return LORAMAC_HANDLER_ERROR;
  }
}

int32_t LmHandlerGetDevAddr( uint32_t *devAddr )
{
  if (devAddr == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *devAddr = CommissioningParams.DevAddr;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetDevAddr( uint32_t devAddr )
{
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    CommissioningParams.DevAddr = devAddr;
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change DevAddr in running state
    return LORAMAC_HANDLER_ERROR;
  }
}

int32_t LmHandlerSetNwkSKey( uint8_t* nwkSKey )
{
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    UTIL_MEM_cpy_8( CommissioningParams.NwkSEncKey, nwkSKey, 16 );
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change Keys in running state
    return LORAMAC_HANDLER_ERROR;
  }
#else /* LORAWAN_KMS == 1 */
#if defined (ACTIVATION_BY_PERSONALISATION) && (ACTIVATION_BY_PERSONALISATION == 1) 
  /* this feature is currently not supported by KMS */
  /* Ticket 80993: The function CreateObject will be used when available on Kms */
  /* It will create an key in NVM and remap the init table on that handler */
#endif
  return LORAMAC_HANDLER_ERROR;
#endif  /* KMS */
}

int32_t LmHandlerSetAppSKey( uint8_t* appSKey )
{
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    UTIL_MEM_cpy_8( CommissioningParams.AppSKey, appSKey, 16 );
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change Keys in running state
    return LORAMAC_HANDLER_ERROR;
  }
#else /* LORAWAN_KMS == 1 */
#if defined (ACTIVATION_BY_PERSONALISATION) && (ACTIVATION_BY_PERSONALISATION == 1) 
  /* this feature is currently not supported by KMS */
  /* Ticket 80993: The function CreateObject will be used when available on Kms */
  /* It will create an key in NVM and remap the init table on that handler */
#endif
  return LORAMAC_HANDLER_ERROR;
#endif  /* KMS */
}

int32_t LmHandlerGetActiveRegion( LoRaMacRegion_t *region )
{
  if (region == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *region = LmHandlerParams.ActiveRegion;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetActiveRegion( LoRaMacRegion_t region )
{
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    LmHandlerParams.ActiveRegion = region;
    return LmHandlerConfigure(&LmHandlerParams);
  }
  else
  {
    // Cannot change Region in running state
    return LORAMAC_HANDLER_ERROR;
  }
}

int32_t LmHandlerGetAdrEnable( bool *adrEnable )
{
  if (adrEnable == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *adrEnable = LmHandlerParams.AdrEnable;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetAdrEnable( bool adrEnable )
{
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_ADR;
  mibReq.Param.AdrEnable = adrEnable;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  LmHandlerParams.AdrEnable = adrEnable;

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetTxDatarate( int8_t *txDatarate )
{
  if (txDatarate == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *txDatarate = LmHandlerParams.TxDatarate;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetTxDatarate( int8_t txDatarate )
{
  if (LmHandlerParams.AdrEnable == true)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_CHANNELS_DATARATE;
  mibReq.Param.ChannelsDatarate = txDatarate;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  LmHandlerParams.TxDatarate = txDatarate;
  
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetDutyCycleEnable( bool *dutyCycleEnable )
{
  if (dutyCycleEnable == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *dutyCycleEnable = LmHandlerParams.DutyCycleEnabled;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetDutyCycleEnable( bool dutyCycleEnable )
{
  LmHandlerParams.DutyCycleEnabled = dutyCycleEnable;
  LoRaMacTestSetDutyCycleOn(dutyCycleEnable);

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetRX2Params( RxChannelParams_t *rxParams )
{
  if (rxParams == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  MibRequestConfirm_t mibReq;
  
  mibReq.Type = MIB_RX2_CHANNEL;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  rxParams->Frequency = mibReq.Param.Rx2Channel.Frequency;
  rxParams->Datarate = mibReq.Param.Rx2Channel.Datarate;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetTxPower( int8_t *txPower )
{
  MibRequestConfirm_t mibReq;
  if (txPower == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mibReq.Type = MIB_CHANNELS_TX_POWER;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *txPower = mibReq.Param.ChannelsTxPower;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetRx1Delay( uint32_t *rxDelay )
{
  MibRequestConfirm_t mibReq;
  if (rxDelay == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mibReq.Type = MIB_RECEIVE_DELAY_1;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *rxDelay = mibReq.Param.ReceiveDelay1;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetRx2Delay( uint32_t *rxDelay )
{
  MibRequestConfirm_t mibReq;
  if (rxDelay == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mibReq.Type = MIB_RECEIVE_DELAY_2;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *rxDelay = mibReq.Param.ReceiveDelay2;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetJoinRx1Delay( uint32_t *rxDelay )
{
  MibRequestConfirm_t mibReq;
  if (rxDelay == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mibReq.Type = MIB_JOIN_ACCEPT_DELAY_1;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *rxDelay = mibReq.Param.JoinAcceptDelay1;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetJoinRx2Delay( uint32_t *rxDelay )
{
  MibRequestConfirm_t mibReq;
  if (rxDelay == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mibReq.Type = MIB_JOIN_ACCEPT_DELAY_2;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *rxDelay = mibReq.Param.JoinAcceptDelay2;
  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetTxPower(int8_t txPower)
{
  MibRequestConfirm_t mibReq;

  mibReq.Type = MIB_CHANNELS_TX_POWER;
  mibReq.Param.ChannelsTxPower = txPower;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetRX2Params( RxChannelParams_t *rxParams)
{
  MibRequestConfirm_t mibReq;
  
  mibReq.Type = MIB_RX2_CHANNEL;
  mibReq.Param.Rx2Channel.Frequency = rxParams->Frequency;
  mibReq.Param.Rx2Channel.Datarate = rxParams->Datarate;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetRx1Delay( uint32_t rxDelay )
{
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_RECEIVE_DELAY_1;
  mibReq.Param.ReceiveDelay1 = rxDelay;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetRx2Delay( uint32_t rxDelay )
{
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_RECEIVE_DELAY_2;
  mibReq.Param.ReceiveDelay2 = rxDelay;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetJoinRx1Delay( uint32_t rxDelay )
{
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_JOIN_ACCEPT_DELAY_1;
  mibReq.Param.JoinAcceptDelay1 = rxDelay;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerSetJoinRx2Delay( uint32_t rxDelay )
{
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_JOIN_ACCEPT_DELAY_2;
  mibReq.Param.JoinAcceptDelay2 = rxDelay;
  if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  return LORAMAC_HANDLER_SUCCESS;
}

int32_t LmHandlerGetPingPeriodicity( uint8_t *pingPeriodicity )
{
#if ( LORAMAC_CLASSB_ENABLED == 1 )
  if (pingPeriodicity == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }

  *pingPeriodicity = LmHandlerParams.PingPeriodicity;
  return LORAMAC_HANDLER_SUCCESS;
#else
  return LORAMAC_HANDLER_ERROR;
#endif
}

int32_t LmHandlerSetPingPeriodicity( uint8_t pingPeriodicity )
{
#if ( LORAMAC_CLASSB_ENABLED == 1 )
  // Not yet joined
  if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
  {
    LmHandlerParams.PingPeriodicity = pingPeriodicity;
    return LORAMAC_HANDLER_SUCCESS;
  }
  else
  {
    // Cannot change Region in running state
    return LmHandlerPingSlotReq(pingPeriodicity);
  }
#else
  return LORAMAC_HANDLER_ERROR;
#endif
}

int32_t LmHandlerGetBeaconState( BeaconState_t *beaconState )
{
#if ( LORAMAC_CLASSB_ENABLED == 1 )
  MibRequestConfirm_t mibReq;
  LoRaMacClassBNvmCtx_t* CtxClassB;
  
  if (beaconState == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  mibReq.Type = MIB_NVM_CTXS;
  if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_ERROR;
  }
  CtxClassB = ( LoRaMacClassBNvmCtx_t*) mibReq.Param.Contexts->ClassBNvmCtx;
  if (CtxClassB == NULL)
  {
    return LORAMAC_HANDLER_ERROR;
  }
  
  *beaconState =  CtxClassB->BeaconCtx.BeaconState;
  return LORAMAC_HANDLER_SUCCESS;
#else
  return LORAMAC_HANDLER_ERROR;
#endif
}

/* Private  functions ---------------------------------------------------------*/
static LmHandlerFlagStatus_t LmHandlerJoinStatus( void )
{
  MibRequestConfirm_t mibReq;
  LoRaMacStatus_t status;
  
  mibReq.Type = MIB_NETWORK_ACTIVATION;
  status = LoRaMacMibGetRequestConfirm( &mibReq );
  
  if( status == LORAMAC_STATUS_OK )
  {
    if( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE )
    {
      return LORAMAC_HANDLER_RESET;
    }
    else
    {
      return LORAMAC_HANDLER_SET;
    }
  }
  else
  {
    return LORAMAC_HANDLER_RESET;
  }
}

static LmHandlerErrorStatus_t LmHandlerDeviceTimeReq( void )
{
  LoRaMacStatus_t status;
  MlmeReq_t mlmeReq;
  
  mlmeReq.Type = MLME_DEVICE_TIME;
  
  TimerTime_t nextTxIn = 0;
  LoRaMacQueryNextTxDelay( TxParams.Datarate, &nextTxIn );
  status = LoRaMacMlmeRequest( &mlmeReq );

  if( status == LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_SUCCESS;
  }
  else
  {
    return LORAMAC_HANDLER_ERROR;
  }
}

#if ( LORAMAC_CLASSB_ENABLED == 1 )
static LmHandlerErrorStatus_t LmHandlerBeaconReq( void )
{
  LoRaMacStatus_t status = LORAMAC_STATUS_OK;
  MlmeReq_t mlmeReq;

  mlmeReq.Type = MLME_BEACON_ACQUISITION;

  TimerTime_t nextTxIn = 0;
  LoRaMacQueryNextTxDelay( TxParams.Datarate, &nextTxIn );
  status = LoRaMacMlmeRequest( &mlmeReq );
  
  if( status == LORAMAC_STATUS_OK )
  {
    return LORAMAC_HANDLER_SUCCESS;
  }
  else
  {
    return LORAMAC_HANDLER_ERROR;
  }
}

static LmHandlerErrorStatus_t LmHandlerPingSlotReq( uint8_t periodicity )
{
  LoRaMacStatus_t status;
  MlmeReq_t mlmeReq;
  
  mlmeReq.Type = MLME_PING_SLOT_INFO;
  mlmeReq.Req.PingSlotInfo.PingSlot.Fields.Periodicity = periodicity;
  mlmeReq.Req.PingSlotInfo.PingSlot.Fields.RFU = 0;
  
  TimerTime_t nextTxIn = 0;
  LoRaMacQueryNextTxDelay( TxParams.Datarate, &nextTxIn );
  status = LoRaMacMlmeRequest( &mlmeReq );
  
  if( status == LORAMAC_STATUS_OK )
  {
    LmHandlerParams.PingPeriodicity = periodicity;
    // Send an empty message
    LmHandlerAppData_t appData =
    {
      .Buffer = NULL,
      .BufferSize = 0,
      .Port = 0
    };
    return LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, &nextTxIn, false );
  }
  else
  {
    return LORAMAC_HANDLER_ERROR;
  }
}
#endif

static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
  TxParams.IsMcpsConfirm = 1;
  TxParams.Status = mcpsConfirm->Status;
  TxParams.Datarate = mcpsConfirm->Datarate;
  TxParams.UplinkCounter = mcpsConfirm->UpLinkCounter;
  TxParams.TxPower = mcpsConfirm->TxPower;
  TxParams.Channel = mcpsConfirm->Channel;
  TxParams.AckReceived = mcpsConfirm->AckReceived;
  
  LmHandlerCallbacks.OnTxData( &TxParams );
  
  LmHandlerPackagesNotify( PACKAGE_MCPS_CONFIRM, mcpsConfirm );
}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
  LmHandlerAppData_t appData;
  DeviceClass_t deviceClass;
  RxParams.IsMcpsIndication = 1;
  RxParams.Status = mcpsIndication->Status;
  
  if( RxParams.Status != LORAMAC_EVENT_INFO_STATUS_OK )
  {
    return;
  }
  
  if (mcpsIndication->BufferSize > 0)
  {
    RxParams.Datarate = mcpsIndication->RxDatarate;
    RxParams.Rssi = mcpsIndication->Rssi;
    RxParams.Snr = mcpsIndication->Snr;
    RxParams.DownlinkCounter = mcpsIndication->DownLinkCounter;
    RxParams.RxSlot = mcpsIndication->RxSlot;
    
    appData.Port = mcpsIndication->Port;
    appData.BufferSize = mcpsIndication->BufferSize;
    appData.Buffer = mcpsIndication->Buffer;

    LmHandlerCallbacks.OnRxData( &appData, &RxParams );
  }

  // Call packages RxProcess function
  LmHandlerPackagesNotify( PACKAGE_MCPS_INDICATION, mcpsIndication );
  LmHandlerGetCurrentClass(&deviceClass);
  if( ( mcpsIndication->FramePending == true ) && ( deviceClass == CLASS_A ) )
  {
    // The server signals that it has pending data to be sent.
    // We schedule an uplink as soon as possible to flush the server.
    
    // Send an empty message
    LmHandlerAppData_t appData =
    {
      .Buffer = NULL,
      .BufferSize = 0,
      .Port = 0
    };
    LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true );
  }
}

static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
  TxParams.IsMcpsConfirm = 0;
  TxParams.Status = mlmeConfirm->Status;
  
  LmHandlerPackagesNotify( PACKAGE_MLME_CONFIRM, mlmeConfirm );
  
  switch( mlmeConfirm->MlmeRequest )
  {
  case MLME_JOIN:
    {
      MibRequestConfirm_t mibReq;
      mibReq.Type = MIB_DEV_ADDR;
      LoRaMacMibGetRequestConfirm( &mibReq );
      CommissioningParams.DevAddr = mibReq.Param.DevAddr;
      LmHandlerGetTxDatarate(&JoinParams.Datarate);
      
      if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
      {
        // Status is OK, node has joined the network
        JoinParams.Status = LORAMAC_HANDLER_SUCCESS;

        // Notify upper layer
        LmHandlerRequestClass( LmHandlerParams.DefaultClass );
      }
      else
      {
        // Join was not successful. Try to join again
        JoinParams.Status = LORAMAC_HANDLER_ERROR;
      }
      LmHandlerCallbacks.OnJoinRequest( &JoinParams );
    }
    break;
  case MLME_LINK_CHECK:
    {
      // Check DemodMargin
      // Check NbGateways
    }
    break;
  case MLME_DEVICE_TIME:
    {
#if ( LORAMAC_CLASSB_ENABLED == 1 )
      if( IsClassBSwitchPending == true )
      {
        LmHandlerBeaconReq( );
      }
#endif
    }
    break;
#if ( LORAMAC_CLASSB_ENABLED == 1 )
  case MLME_BEACON_ACQUISITION:
    {
      if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
      {
        // Beacon has been acquired
        // Request server for ping slot
        LmHandlerPingSlotReq( LmHandlerParams.PingPeriodicity );
      }
      else
      {
        // Beacon not acquired
        // Request Device Time again.
        LmHandlerDeviceTimeReq( );
      }
    }
    break;
  case MLME_PING_SLOT_INFO:
    {
      if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
      {
        MibRequestConfirm_t mibReq;
        
        // Class B is now activated
        mibReq.Type = MIB_DEVICE_CLASS;
        mibReq.Param.Class = CLASS_B;
        LoRaMacMibSetRequestConfirm( &mibReq );
        
        DisplayClassUpdate(CLASS_B);

        IsClassBSwitchPending = false;
      }
      else
      {
        LmHandlerPingSlotReq( LmHandlerParams.PingPeriodicity );
      }
    }
    break;
#endif
  default:
    break;
  }
}

static void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
  RxParams.IsMcpsIndication = 0;
  RxParams.Status = mlmeIndication->Status;
  
  switch( mlmeIndication->MlmeIndication )
  {
#if ( LORAMAC_CLASSB_ENABLED == 1 )
  case MLME_BEACON_LOST:
    {
      MibRequestConfirm_t mibReq;
      // Switch to class A again
      mibReq.Type = MIB_DEVICE_CLASS;
      mibReq.Param.Class = CLASS_A;
      LoRaMacMibSetRequestConfirm( &mibReq );
      
      BeaconParams.State = LORAMAC_HANDLER_BEACON_LOST;
      BeaconParams.Info.Time.Seconds = 0;
      BeaconParams.Info.GwSpecific.InfoDesc = 0;
      UTIL_MEM_set_8( BeaconParams.Info.GwSpecific.Info, 0, 6 );
      
      DisplayClassUpdate(CLASS_A);
      DisplayBeaconUpdate( &BeaconParams );
      
      LmHandlerDeviceTimeReq( );
    }
    break;
  case MLME_BEACON:
    {
      if( mlmeIndication->Status == LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED )
      {
        BeaconParams.State = LORAMAC_HANDLER_BEACON_RX;
        BeaconParams.Info = mlmeIndication->BeaconInfo;
        
        DisplayBeaconUpdate( &BeaconParams );
      }
      else
      {
        BeaconParams.State = LORAMAC_HANDLER_BEACON_NRX;
        BeaconParams.Info = mlmeIndication->BeaconInfo;
        
        DisplayBeaconUpdate( &BeaconParams );
      }
      break;
    }
#endif
  default:
    break;
  }
}

static bool LmHandlerPackageIsInitialized( uint8_t id )
{
  if( LmHandlerPackages[id]->IsInitialized != NULL )
  {
    return LmHandlerPackages[id]->IsInitialized( );
  }
  else
  {
    return false;
  }
}

static void LmHandlerPackagesNotify( PackageNotifyTypes_t notifyType, void *params )
{
  for( int8_t i = 0; i < PKG_MAX_NUMBER; i++ )
  {
    if( LmHandlerPackages[i] != NULL )
    {
      switch( notifyType )
      {
      case PACKAGE_MCPS_CONFIRM:
        {
          if( LmHandlerPackages[i]->OnMcpsConfirmProcess != NULL )
          {
            LmHandlerPackages[i]->OnMcpsConfirmProcess( params );
          }
          break;
        }
      case PACKAGE_MCPS_INDICATION:
        {
          if( ( LmHandlerPackages[i]->OnMcpsIndicationProcess != NULL ) &&
              (( LmHandlerPackages[i]->Port == ( ( McpsIndication_t* )params )->Port ) ||
               ( (i == PACKAGE_ID_COMPLIANCE ) && ( LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->IsRunning( ) ))))
          {
            LmHandlerPackages[i]->OnMcpsIndicationProcess( params );
          }
          break;
        }
      case PACKAGE_MLME_CONFIRM:
        {
          if( LmHandlerPackages[i]->OnMlmeConfirmProcess != NULL )
          {
            LmHandlerPackages[i]->OnMlmeConfirmProcess( params );
          }
          break;
        }
      default:
        break;
      }
    }
  }
}

static void DisplayClassUpdate( DeviceClass_t deviceClass )
{
  MW_LOG(TS_OFF, VLEVEL_M, "Switch to Class %c done\r\n", "ABC"[deviceClass] );
}

#if ( LORAMAC_CLASSB_ENABLED == 1 )
static void DisplayBeaconUpdate( LoRaMAcHandlerBeaconParams_t *params )
{
  static const char* EventBeaconStateStrings[] = { "BC_ACQUIRING", "BC_LOST", "BC_RECEIVED", "BC_NOT_RECEIVED" };

  MW_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== %s\r\n", EventBeaconStateStrings[params->State] );
  if (params->State == LORAMAC_HANDLER_BEACON_RX)
  {
    MW_LOG(TS_OFF, VLEVEL_H, "###### BTIME:%010d | GW DESC:%d | GW INFO:%02X %02X %02X %02X %02X %02X\r\n", params->Info.Time.Seconds, params->Info.GwSpecific.InfoDesc,
           params->Info.GwSpecific.Info[0], params->Info.GwSpecific.Info[1],
           params->Info.GwSpecific.Info[2], params->Info.GwSpecific.Info[3],
           params->Info.GwSpecific.Info[4], params->Info.GwSpecific.Info[5]);
    MW_LOG(TS_OFF, VLEVEL_H, "###### FREQ:%d | DR:%d | RSSI:%d | SNR:%d\r\n", params->Info.Frequency, params->Info.Datarate, params->Info.Rssi, params->Info.Snr );
  }
}
#endif
