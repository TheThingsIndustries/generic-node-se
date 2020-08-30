/*!
 * \file      LmHandler.h
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
  * @file    LmHandler.h
  * @author  MCD Application Team
  * @brief   Header for LoRaMAC Layer handling module
  ******************************************************************************
  */
#ifndef __LORAMAC_HANDLER_H__
#define __LORAMAC_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LmHandlerTypes.h"
#include "LoRaMacClassB.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct LmHandlerJoinParams_s
{
  int8_t Datarate;
  LmHandlerErrorStatus_t Status;
  ActivationType_t Mode;
}LmHandlerJoinParams_t;

typedef struct LmHandlerTxParams_s
{
  uint8_t IsMcpsConfirm;
  LoRaMacEventInfoStatus_t Status;
  LmHandlerMsgTypes_t MsgType;
  uint8_t AckReceived;
  int8_t Datarate;
  uint32_t UplinkCounter;
  LmHandlerAppData_t AppData;
  int8_t TxPower;
  uint8_t Channel;
}LmHandlerTxParams_t;

typedef struct LmHandlerRxParams_s
{
  uint8_t IsMcpsIndication;
  LoRaMacEventInfoStatus_t Status;
  int8_t Datarate;
  int8_t Rssi;
  int8_t Snr;
  uint32_t DownlinkCounter;
  int8_t RxSlot;
}LmHandlerRxParams_t;

typedef struct LoRaMAcHandlerBeaconParams_s
{
  LoRaMacEventInfoStatus_t Status;
  LmHandlerBeaconState_t State;
  BeaconInfo_t Info;
}LoRaMAcHandlerBeaconParams_t;

typedef struct LmHandlerParams_s
{
  /*!
   * Active Region
   */
  LoRaMacRegion_t ActiveRegion;
  /*!
   * Default Class
   */
  DeviceClass_t DefaultClass;
  /*!
   * Holds the ADR state
   */
  bool AdrEnable;
  /*!
   * Uplink datarate, when \ref AdrEnable is OFF
   */
  int8_t TxDatarate;
  /*!
   * LoRaWAN ETSI duty cycle control enable/disable
   *
   * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
   */
  bool DutyCycleEnabled;
  /*!
   * Periodicity of the ping slots
   */
  uint8_t PingPeriodicity;
}LmHandlerParams_t;

typedef struct LmHandlerCallbacks_s
{
  /*!
   * Get the current battery level
   *
   * \retval value  Battery level ( 0: very low, 254: fully charged )
   */
  uint8_t ( *GetBatteryLevel )( void );
  /*!
   * Get the current temperature
   *
   * \retval value  Temperature in degree Celsius
   */
  /*float*/ uint16_t ( *GetTemperature )( void );
  /*!
   * Gets the board 64 bits unique ID
   *
   * \param [IN] id Pointer to an array that will contain the Unique ID
   */
  void ( *GetUniqueId )( uint8_t *id );
  /*!
   * Returns a pseudo random seed generated using the MCU Unique ID
   *
   * \retval seed Generated pseudo random seed
   */
  uint32_t ( *GetRandomSeed )( void );
  /*!
   * Notifies the upper layer that a network has been joined
   *
   * \param [IN] params notification parameters
   */
  void ( *OnJoinRequest )( LmHandlerJoinParams_t *params );
  /*!
   * Notifies upper layer that a frame has been transmitted
   *
   * \param [IN] params notification parameters
   */
  void ( *OnTxData )( LmHandlerTxParams_t *params );
  /*!
   * Notifies the upper layer that an applicative frame has been received
   *
   * \param [IN] appData Received applicative data
   * \param [IN] params notification parameters
   */
  void ( *OnRxData )( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params );
  /*!
   *\brief    Will be called each time a Radio IRQ is handled by the MAC
   *          layer.
   * 
   *\warning  Runs in a IRQ context. Should only change variables state.
   */
  void ( *OnMacProcess )( void );
}LmHandlerCallbacks_t;

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/*!
 * LoRaMac handler initialisation
 *
 * \param [IN] callbacks     LoRaMac handler callbacks
 *
 * \retval none
 */
LmHandlerErrorStatus_t LmHandlerInit( LmHandlerCallbacks_t *handlerCallbacks );

/*!
 * Indicates if the LoRaMacHandler is busy
 * 
 * \retval status [true] Busy, [false] free
 */
bool LmHandlerIsBusy( void );

/*!
 * Instructs the MAC layer to send a ClassA uplink
 *
 * \param [IN] appData Data to be sent
 * \param [IN] isTxConfirmed Indicates if the uplink requires an acknowledgement
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SUCCESS if request has been
 *                processed else \ref LORAMAC_HANDLER_ERROR
 */
LmHandlerErrorStatus_t LmHandlerSend( LmHandlerAppData_t *appData, LmHandlerMsgTypes_t isTxConfirmed, TimerTime_t *nextTxIn, bool allowDelayedTx );

/*!
 * Join a LoRa Network in classA
 *
 * \Note if the device is ABP, this is a pass through function
 *
 * \retval none
 */
void LmHandlerJoin( ActivationType_t mode );

/*!
 * Stop a LoRa Network connection
 *
 * \retval none
 */
void LmHandlerStop( void );




/*!
 * Request the MAC layer to change LoRaWAN class
 *
 * \Note Callback \ref LmHandlerConfirmClass informs upper layer that the change has occurred
 * \Note Only switch from class A to class B/C OR from class B/C to class A is allowed
 *
 * \param [IN] newClass New class to be requested
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SUCCESS if request has been
 *                processed else \ref LORAMAC_HANDLER_ERROR
 */
LmHandlerErrorStatus_t LmHandlerRequestClass( DeviceClass_t newClass );

/*!
 * LoRaMac handler configuration
 * \param [IN] handlerParams LoRaMac handler parameters
 * \retval none
 */
LmHandlerErrorStatus_t LmHandlerConfigure( LmHandlerParams_t *handlerParams );

LmHandlerErrorStatus_t LmHandlerPackageRegister( uint8_t id, void *params );

void LmHandlerPackagesProcess( void );

/**
  * @brief  Gets the current LoRaWAN class
  * @param  current class
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetCurrentClass( DeviceClass_t *deviceClass );

/**
  * @brief  Gets the LoRaWAN Device EUI (if OTAA)
  * @param  LoRaWAN DevEUI
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetDevEUI( uint8_t *devEUI );

/**
  * @brief  Sets the LoRaWAN Device EUI (if OTAA)
  * @param  LoRaWAN DevEUI
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetDevEUI( uint8_t *devEUI );

/**
  * @brief  Gets the LoRaWAN AppEUI (from the Commissionning)
  * @param LoRaWAN AppEUI
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetAppEUI( uint8_t *appEUI );

/**
  * @brief  Sets the LoRaWAN App EUI (if OTAA)
  * @param  LoRaWAN AppEUI
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetAppEUI( uint8_t *appEUI );

/**
  * @brief  Gets the LoRaWAN AppKey (from the Commissionning)
  * @param LoRaWAN AppKey
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetAppKey( uint8_t *appKey );

/**
  * @brief  Sets the LoRaWAN AppKey (if OTAA)
  * @param  LoRaWAN AppKey
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetAppKey( uint8_t *appKey );

/**
  * @brief  Gets the LoRaWAN Network ID (from the Commissionning if ABP or after the Join if OTAA)
  * @param  networkId
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetNetworkID( uint32_t *networkId );

/**
  * @brief  Sets the LoRaWAN Network ID
  * @param  networkId
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetNetworkID( uint32_t networkId );

/**
  * @brief  Gets the LoRaWAN Device Address (from the Commissionning if ABP or after the Join if OTAA)
  * @param  device address
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetDevAddr( uint32_t *devAddr );

/**
  * @brief  Sets the LoRaWAN Device Address (if ABP)
  * @param  device address
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetDevAddr( uint32_t devAddr );

/**
  * @brief  Sets the LoRaWAN Network Session Key (if ABP)
  * @param  network session key
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetNwkSKey( uint8_t* nwkSKey );

/**
  * @brief  Sets the LoRaWAN Network Application Key (if ABP)
  * @param  application session key
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetAppSKey( uint8_t* appSKey );

/**
  * @brief  Gets the current active region
  * @param Current active region
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetActiveRegion( LoRaMacRegion_t *region );

/**
  * @brief  Sets the current active region
  * @param Current active region
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetActiveRegion( LoRaMacRegion_t region );

/**
  * @brief  Gets the Adaptive data rate (1 = the Network manages the DR, 0 = the device manages the DR)
  * @param  Adaptive data rate flag
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetAdrEnable( bool *adrEnable );

/**
  * @brief  Sets the Adaptive data rate (1 = the Network manages the DR, 0 = the device manages the DR)
  * @param  Adaptive data rate flag
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetAdrEnable( bool adrEnable );

/**
  * @brief  Gets the current datarate
  * @param  Current datarate
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetTxDatarate( int8_t *txDatarate );

/**
  * @brief  Sets the current datarate
  * @param  Current datarate
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetTxDatarate( int8_t txDatarate );

/**
  * @brief  Gets the duty cycle flag
  * @param  duty cycle flag
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetDutyCycleEnable( bool *dutyCycleEnable );

/**
  * @brief  Sets the current datarate
  * @param  Current datarate
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetDutyCycleEnable( bool dutyCycleEnable );

/**
  * @brief  Gets the current RX_2 datarate and frequency
  * @param  rx2 parameters
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerGetRX2Params( RxChannelParams_t *rxParams );

int32_t LmHandlerGetTxPower( int8_t *txPower );

int32_t LmHandlerGetRx1Delay( uint32_t *rxDelay );

int32_t LmHandlerGetRx2Delay( uint32_t *rxDelay );

int32_t LmHandlerGetJoinRx1Delay( uint32_t *rxDelay );

int32_t LmHandlerGetJoinRx2Delay( uint32_t *rxDelay );

/**
  * @brief  Sets the current RX_2 datarate and frequency
  * @param  rx2 parameters
  * @retval -1 LORAMAC_HANDLER_ERROR
  *          0 LORAMAC_HANDLER_SUCCESS
  */
int32_t LmHandlerSetRX2Params( RxChannelParams_t *rxParams);

int32_t LmHandlerSetTxPower( int8_t txPower );

int32_t LmHandlerSetRx1Delay( uint32_t rxDelay );

int32_t LmHandlerSetRx2Delay( uint32_t rxDelay );

int32_t LmHandlerSetJoinRx1Delay( uint32_t rxDelay );

int32_t LmHandlerSetJoinRx2Delay( uint32_t rxDelay );

int32_t LmHandlerGetPingPeriodicity( uint8_t *pingPeriodicity );

int32_t LmHandlerSetPingPeriodicity( uint8_t pingPeriodicity );

int32_t LmHandlerGetBeaconState( BeaconState_t *beaconState );

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_HANDLER_H__
