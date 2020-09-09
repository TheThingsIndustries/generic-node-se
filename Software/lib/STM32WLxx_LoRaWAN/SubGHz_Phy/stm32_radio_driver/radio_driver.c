/*!
 * \file      radio_driver.c
 *
 * \brief     radio driver implementation
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
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
/**
  ******************************************************************************
  *
  *          Portions COPYRIGHT 2020 STMicroelectronics
  *
  * @file    radio_driver.c
  * @author  MCD Application Team
  * @brief   radio driver implementation
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "radio_driver.h" 
#include "radio_conf.h"
#include "mw_log_conf.h"
#include "stm32_lpm.h"
#include "subghz.h"


/*
 * Local types definition
 */
/*!
 * @brief Radio registers definition
 */
typedef struct
{
    uint16_t      Addr;                             //!< The address of the register
    uint8_t       Value;                            //!< The value of the register
}RadioRegisters_t;

typedef enum
{
  RF_API_STATE_TCXO_OFF=0,
  RF_API_STATE_TCXO_ON
} RF_API_Tcxo_State_t;

/*!
 * @brief Holds the internal operating mode of the radio
 */
static RadioOperatingModes_t OperatingMode;

/*!
 * @brief Stores the current packet type set in the radio
 */
static RadioPacketTypes_t PacketType;

/*!
 * \brief Stores the current packet header type set in the radio
 */
static volatile RadioLoRaPacketLengthsMode_t LoRaHeaderType;

/*!
 * @brief Stores the last frequency error measured on LoRa received packet
 */
volatile uint32_t FrequencyError = 0;

/*!
 * @brief Hold the status of the Image calibration
 */
static bool ImageCalibrated = false;


/*
 * Private functions prototypes
 */
/*!
* \This is the workaround for tcxo bug in cut 1.0
*/
static void TCXO_Workaround( RF_API_Tcxo_State_t state );

/*!
* \This is the workaround for Reset bug in cut 1.0
*/
static void RadioReset_Workaround( void );


   
void (*RadioOnDioIrqCb) ( RadioIrqMasks_t radioIrq );

/*
 * Public functions 
 */

uint32_t SUBGRF_GetRadioWakeUpTime( void )
{
  return ( uint32_t ) RBI_GetWakeUpTime();
}

RadioOperatingModes_t SUBGRF_GetOperatingMode( void )
{
  return OperatingMode;
}

int32_t SUBGRF_Init( void (*RadioOnDioIrq) ( RadioIrqMasks_t radioIrq ) )
{
  if ( RadioOnDioIrq!=NULL)
  {
    RadioOnDioIrqCb = RadioOnDioIrq;
  }
  
  TCXO_Workaround(RF_API_STATE_TCXO_ON);
  
  hsubghz.Init.BaudratePrescaler=RADIO_BAUDRATEPRESCALER;
  
  HAL_SUBGHZ_Init(&hsubghz);
  
  ImageCalibrated = false;

  RadioReset_Workaround();
  
  SUBGRF_SetStandby( STDBY_RC );

  if (1U ==RBI_IsTCXO() )
  {
    uint8_t reg = 0x00;
    
    SUBGRF_SetTcxoMode( TCXO_CTRL_1_7V, 64 * RBI_GetWakeUpTime() );// 100 ms
    
    HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_XTA_TRIM, (uint8_t*)&reg , 1);
    
    /*enable calibration for cut1.1 and later*/
    if (LL_DBGMCU_GetRevisionID() !=0x1000)
    {
      CalibrationParams_t calibParam;
      calibParam.Value = 0x7F;
      SUBGRF_Calibrate( calibParam );
    }
  }
  /* Init RF Switch */
  RBI_Init();
  
  OperatingMode = MODE_STDBY_RC;
  
  TCXO_Workaround(RF_API_STATE_TCXO_OFF);
    
  return SUBGRF_OK;
}

void SUBGRF_SetPayload( uint8_t *payload, uint8_t size )
{
    HAL_SUBGHZ_WriteBuffer( &hsubghz, 0x00, payload, size );
}

uint8_t SUBGRF_GetPayload( uint8_t *buffer, uint8_t *size,  uint8_t maxSize )
{
    uint8_t offset = 0;

    SUBGRF_GetRxBufferStatus( size, &offset );
    if( *size > maxSize )
    {
        return 1;
    }
    HAL_SUBGHZ_ReadBuffer( &hsubghz, offset, buffer, *size );
    return 0;
}

void SUBGRF_SendPayload( uint8_t *payload, uint8_t size, uint32_t timeout)
{
    SUBGRF_SetPayload( payload, size );
    SUBGRF_SetTx( timeout );
}

void SUBGRF_SetSwitch (uint8_t power, RFState_t rxtx)
{
  RBI_Switch_TypeDef state= RBI_SWITCH_RX;
  
  if (rxtx == RFSWITCH_TX)
  {
    if (power == RFO_LP)
    {
      state = RBI_SWITCH_RFO_LP;
    }
    if (power == RFO_HP)
    {
      state = RBI_SWITCH_RFO_HP;
    }
  }
  else
  {
    if (rxtx == RFSWITCH_RX)
    {
      state = RBI_SWITCH_RX;
    }
  }
  RBI_ConfigRFSwitch(state);
}

uint8_t SUBGRF_SetSyncWord( uint8_t *syncWord )
{
    HAL_SUBGHZ_WriteRegisters( &hsubghz,REG_LR_SYNCWORDBASEADDRESS, syncWord, 8 );
    return 0;
}

void SUBGRF_SetCrcSeed( uint16_t seed )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( seed >> 8 ) & 0xFF );
    buf[1] = ( uint8_t )( seed & 0xFF );

    switch( SUBGRF_GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_LR_CRCSEEDBASEADDR, buf, 2 );
            break;
        default:
            break;
    }
}

void SUBGRF_SetCrcPolynomial( uint16_t polynomial )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( polynomial >> 8 ) & 0xFF );
    buf[1] = ( uint8_t )( polynomial & 0xFF );

    switch( SUBGRF_GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_LR_CRCPOLYBASEADDR, buf, 2 );
            break;

        default:
            break;
    }
}

void SUBGRF_SetWhiteningSeed( uint16_t seed )
{
  uint8_t regValue = 0;
  
  switch( SUBGRF_GetPacketType( ) )
  {
  case PACKET_TYPE_GFSK:
    HAL_SUBGHZ_ReadRegisters( &hsubghz, REG_LR_WHITSEEDBASEADDR_MSB, &regValue, 1 );
    regValue = regValue & 0xFE;
    regValue = ( ( seed >> 8 ) & 0x01 ) | regValue;
    HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_LR_WHITSEEDBASEADDR_MSB, (uint8_t*)&regValue, 1 ); // only 1 bit.
    HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_LR_WHITSEEDBASEADDR_LSB, (uint8_t*)&seed, 1);
    break;
    
  default:
    break;
  }
}

uint32_t SUBGRF_GetRandom( void )
{
    uint8_t buf[] = { 0, 0, 0, 0 };

    // Set radio in continuous reception
    SUBGRF_SetRfFrequency( 400e6 );
    
    SUBGRF_SetRx( 0 );

    RADIO_DELAY_MS( 1 );

    HAL_SUBGHZ_ReadRegisters( &hsubghz, RANDOM_NUMBER_GENERATORBASEADDR, buf, 4 );

    SUBGRF_SetStandby( STDBY_RC );

    return ( buf[0] << 24 ) | ( buf[1] << 16 ) | ( buf[2] << 8 ) | buf[3];
}

void SUBGRF_SetSleep( SleepParams_t sleepConfig )
{
    RBI_ConfigRFSwitch(RBI_SWITCH_OFF); /* switch the antenna OFF by SW */

    uint8_t value = ( ( ( uint8_t )sleepConfig.Fields.WarmStart << 2 ) |
                      ( ( uint8_t )sleepConfig.Fields.Reset << 1 ) |
                      ( ( uint8_t )sleepConfig.Fields.WakeUpRTC ) );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_SLEEP, &value, 1 );
    OperatingMode = MODE_SLEEP;
    
    TCXO_Workaround(RF_API_STATE_TCXO_OFF);
}

void SUBGRF_SetStandby( RadioStandbyModes_t standbyConfig )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_STANDBY, ( uint8_t* )&standbyConfig, 1 );
    if( standbyConfig == STDBY_RC )
    {
        OperatingMode = MODE_STDBY_RC;
    }
    else
    {
        OperatingMode = MODE_STDBY_XOSC;
    }
}

void SUBGRF_SetFs( void )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_FS, 0, 0 );
    OperatingMode = MODE_FS;
}

void SUBGRF_SetTx( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_TX;

    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_TX, buf, 3 );
}

void SUBGRF_SetRx( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_RX;
  
    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_RX, buf, 3 );
    
}

void SUBGRF_SetRxBoosted( uint32_t timeout )
{
    uint8_t buf[3];
    /* ST_WORKAROUND: Sigfox patch > 0x96 replaced by 0x97 */
    uint8_t reg = 0x97;
    /* ST_WORKAROUND END */
    OperatingMode = MODE_RX;

    HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_RX_GAIN, (uint8_t*)&reg, 1 ); // max LNA gain, increase current by ~2mA for around ~3dB in sensivity

    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_RX, buf, 3 );
}

void SUBGRF_SetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime )
{
    uint8_t buf[6];
    
    buf[0] = ( uint8_t )( ( rxTime >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( rxTime >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( rxTime & 0xFF );
    buf[3] = ( uint8_t )( ( sleepTime >> 16 ) & 0xFF );
    buf[4] = ( uint8_t )( ( sleepTime >> 8 ) & 0xFF );
    buf[5] = ( uint8_t )( sleepTime & 0xFF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_RXDUTYCYCLE, buf, 6 );
    OperatingMode = MODE_RX_DC;
}

void SUBGRF_SetCad( void )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_CAD, 0, 0 );
    
    OperatingMode = MODE_CAD;
}

void SUBGRF_SetTxContinuousWave( void )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_TXCONTINUOUSWAVE, 0, 0 );
}

void SUBGRF_SetTxInfinitePreamble( void )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_TXCONTINUOUSPREAMBLE, 0, 0 );
}

void SUBGRF_SetStopRxTimerOnPreambleDetect( bool enable )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_STOPRXTIMERONPREAMBLE, ( uint8_t* )&enable, 1 );
}

void SUBGRF_SetLoRaSymbNumTimeout( uint8_t SymbNum )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_LORASYMBTIMEOUT, &SymbNum, 1 );
}

void SUBGRF_SetRegulatorMode( void )
{
    RadioRegulatorMode_t mode;
    
    if (1U == RBI_IsDCDC() )
    {
      mode=USE_DCDC ;
    }
    else
    {
       mode=USE_LDO ;
    }
  
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_REGULATORMODE, ( uint8_t* )&mode, 1 );
}

void SUBGRF_Calibrate( CalibrationParams_t calibParam )
{
    uint8_t value = ( ( ( uint8_t )calibParam.Fields.ImgEnable << 6 ) |
                      ( ( uint8_t )calibParam.Fields.ADCBulkPEnable << 5 ) |
                      ( ( uint8_t )calibParam.Fields.ADCBulkNEnable << 4 ) |
                      ( ( uint8_t )calibParam.Fields.ADCPulseEnable << 3 ) |
                      ( ( uint8_t )calibParam.Fields.PLLEnable << 2 ) |
                      ( ( uint8_t )calibParam.Fields.RC13MEnable << 1 ) |
                      ( ( uint8_t )calibParam.Fields.RC64KEnable ) );

    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_CALIBRATE, ( uint8_t* )&value, 1 );
}

void SUBGRF_CalibrateImage( uint32_t freq )
{
    uint8_t calFreq[2];

    if( freq > 900000000 )
    {
        calFreq[0] = 0xE1;
        calFreq[1] = 0xE9;
    }
    else if( freq > 850000000 )
    {
        calFreq[0] = 0xD7;
        calFreq[1] = 0xDB;
    }
    else if( freq > 770000000 )
    {
        calFreq[0] = 0xC1;
        calFreq[1] = 0xC5;
    }
    else if( freq > 460000000 )
    {
        calFreq[0] = 0x75;
        calFreq[1] = 0x81;
    }
    else if( freq > 425000000 )
    {
        calFreq[0] = 0x6B;
        calFreq[1] = 0x6F;
    }
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_CALIBRATEIMAGE, calFreq, 2 );
}

void SUBGRF_SetPaConfig( uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut )
{
    uint8_t buf[4];

    buf[0] = paDutyCycle;
    buf[1] = hpMax;
    buf[2] = deviceSel;
    buf[3] = paLut;
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_PACONFIG, buf, 4 );
}

void SUBGRF_SetRxTxFallbackMode( uint8_t fallbackMode )
{
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_TXFALLBACKMODE, &fallbackMode, 1 );
}

void SUBGRF_SetDioIrqParams( uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask )
{
    uint8_t buf[8];

    buf[0] = ( uint8_t )( ( irqMask >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( irqMask & 0x00FF );
    buf[2] = ( uint8_t )( ( dio1Mask >> 8 ) & 0x00FF );
    buf[3] = ( uint8_t )( dio1Mask & 0x00FF );
    buf[4] = ( uint8_t )( ( dio2Mask >> 8 ) & 0x00FF );
    buf[5] = ( uint8_t )( dio2Mask & 0x00FF );
    buf[6] = ( uint8_t )( ( dio3Mask >> 8 ) & 0x00FF );
    buf[7] = ( uint8_t )( dio3Mask & 0x00FF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_CFG_DIOIRQ, buf, 8 );
}

uint16_t SUBGRF_GetIrqStatus( void )
{
    uint8_t irqStatus[2];

    HAL_SUBGHZ_ExecGetCmd( &hsubghz, RADIO_GET_IRQSTATUS, irqStatus, 2 );
    return ( irqStatus[0] << 8 ) | irqStatus[1];
}

void SUBGRF_SetTcxoMode (RadioTcxoCtrlVoltage_t tcxoVoltage, uint32_t timeout )
{
    uint8_t buf[4];

    buf[0] = tcxoVoltage & 0x07;
    buf[1] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( timeout & 0xFF );

    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_TCXOMODE, buf, 4 );
}

void SUBGRF_SetRfFrequency( uint32_t frequency )
{
    uint8_t buf[4];
    uint32_t chan;
    
    TCXO_Workaround(RF_API_STATE_TCXO_ON);
    
    if( ImageCalibrated == false )
    {
        SUBGRF_CalibrateImage( frequency );
        ImageCalibrated = true;
    }

    //chan = ( uint32_t )( ( double )frequency / ( double )FREQ_STEP );
    SX_FREQ_TO_CHANNEL(chan, frequency);   

    buf[0] = ( uint8_t )( ( chan >> 24 ) & 0xFF );
    buf[1] = ( uint8_t )( ( chan >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( chan >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( chan & 0xFF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_RFFREQUENCY, buf, 4 );
    
    
}

void SUBGRF_SetPacketType( RadioPacketTypes_t packetType )
{
  uint8_t reg = 0x00;
    // Save packet type internally to avoid questioning the radio
    PacketType = packetType;
    
    if( packetType == PACKET_TYPE_GFSK )
    {
        HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_BIT_SYNC, (uint8_t*)&reg, 1 );
    }
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_PACKETTYPE, ( uint8_t* )&packetType, 1 );
}

RadioPacketTypes_t SUBGRF_GetPacketType( void )
{
    return PacketType;
}

uint8_t SUBGRF_SetTxPower(  int8_t power, RadioRampTimes_t rampTime ) 
{
  uint8_t paSelect= RFO_LP;
  
  int32_t TxConfig = RBI_GetTxConfig();
  
  switch (TxConfig)
  {
    case RBI_CONF_RFO_LP_HP:
    {
      if (power>15)
      {
        paSelect= RFO_HP;
      }
      else
      {
        paSelect= RFO_LP;
      }
      break;
    }
    case RBI_CONF_RFO_LP:
    {
      paSelect= RFO_LP;
      break;
    }
    case RBI_CONF_RFO_HP:
    {
      paSelect= RFO_HP;
      break;
    }
  default:
    break;
  }

  SUBGRF_SetTxParams(  paSelect,  power,  rampTime );
  
  return paSelect;
}
void SUBGRF_SetTxParams( uint8_t paSelect, int8_t power, RadioRampTimes_t rampTime ) 
{
    uint8_t buf[2];
    uint8_t reg;

    if( paSelect == RFO_LP )
    {
        if( power == 15 )
        {
            SUBGRF_SetPaConfig( 0x06, 0x00, 0x01, 0x01 );
        }
        else
        {
            SUBGRF_SetPaConfig( 0x04, 0x00, 0x01, 0x01 );
        }
        if( power >= 14 )
        {
            power = 14;
        }
        else if( power < -17 )
        {
            power = -17;
        }
        reg = 0x18;
        HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_OCP, (uint8_t*)&reg, 1 ); // current max is 80 mA for the whole device
    }
    else // rfo_hp
    {
        // WORKAROUND - Better Resistance of the SX1262 Tx to Antenna Mismatch, see DS_SX1261-2_V1.2 datasheet chapter 15.2
        // RegTxClampConfig = @address 0x08D8
        HAL_SUBGHZ_ReadRegisters( &hsubghz, REG_TX_CLAMP, &reg, 1 );
        reg |= ( 0x0F << 1 );
        HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_TX_CLAMP, &reg, 1 );
        // WORKAROUND END

        SUBGRF_SetPaConfig( 0x04, 0x07, 0x00, 0x01 );
        if( power > 22 )
        {
            power = 22;
        }
        else if( power < -9 )
        {
            power = -9;
        }
        reg = 0x38;
        HAL_SUBGHZ_WriteRegisters( &hsubghz, REG_OCP, (uint8_t*)&reg, 1 ); // current max 160mA for the whole device
    }
    buf[0] = power;
    buf[1] = ( uint8_t )rampTime;
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_TXPARAMS, buf, 2 );
}



void SUBGRF_SetModulationParams( ModulationParams_t *modulationParams )
{
    uint8_t n;
    uint32_t tempVal = 0;
    uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // Check if required configuration corresponds to the stored packet type
    // If not, silently update radio packet type
    if( PacketType != modulationParams->PacketType )
    {
        SUBGRF_SetPacketType( modulationParams->PacketType );
    }

    switch( modulationParams->PacketType )
    {
    case PACKET_TYPE_GFSK:
        n = 8;
        tempVal = ( uint32_t )( 32 * ( ( double )XTAL_FREQ / ( double )modulationParams->Params.Gfsk.BitRate ) );
        buf[0] = ( tempVal >> 16 ) & 0xFF;
        buf[1] = ( tempVal >> 8 ) & 0xFF;
        buf[2] = tempVal & 0xFF;
        buf[3] = modulationParams->Params.Gfsk.ModulationShaping;
        buf[4] = modulationParams->Params.Gfsk.Bandwidth;
        //tempVal = ( uint32_t )( ( double )modulationParams->Params.Gfsk.Fdev / ( double )FREQ_STEP );
        SX_FREQ_TO_CHANNEL(tempVal, modulationParams->Params.Gfsk.Fdev); 
        buf[5] = ( tempVal >> 16 ) & 0xFF;
        buf[6] = ( tempVal >> 8 ) & 0xFF;
        buf[7] = ( tempVal& 0xFF );
        HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_MODULATIONPARAMS, buf, n );
        break;
    case PACKET_TYPE_BPSK:
        n = 4;
        tempVal = ( uint32_t )( 32 * ( ( double )XTAL_FREQ / ( double )modulationParams->Params.Bpsk.BitRate ) );

        buf[0] = ( tempVal >> 16 ) & 0xFF;
        buf[1] = ( tempVal >> 8 ) & 0xFF;
        buf[2] = tempVal & 0xFF;
        buf[3] = modulationParams->Params.Bpsk.ModulationShaping;
        HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_MODULATIONPARAMS, buf, n );
        break;
    case PACKET_TYPE_LORA:
        n = 4;
        buf[0] = modulationParams->Params.LoRa.SpreadingFactor;
        buf[1] = modulationParams->Params.LoRa.Bandwidth;
        buf[2] = modulationParams->Params.LoRa.CodingRate;
        buf[3] = modulationParams->Params.LoRa.LowDatarateOptimize;

        HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_MODULATIONPARAMS, buf, n );

        break;
    case PACKET_TYPE_GMSK:
        n = 5;
        tempVal = ( uint32_t )( 32 * ( ( double )XTAL_FREQ / ( double )modulationParams->Params.Gfsk.BitRate ) );
        buf[0] = ( tempVal >> 16 ) & 0xFF;
        buf[1] = ( tempVal >> 8 ) & 0xFF;
        buf[2] = tempVal & 0xFF;
        buf[3] = modulationParams->Params.Gfsk.ModulationShaping;
        buf[4] = modulationParams->Params.Gfsk.Bandwidth;
        HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_MODULATIONPARAMS, buf, n );
        break;
    default:
    case PACKET_TYPE_NONE:
      break;
    }
}

void SUBGRF_SetPacketParams( PacketParams_t *packetParams )
{
    uint8_t n;
    uint8_t crcVal = 0;
    uint8_t buf[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // Check if required configuration corresponds to the stored packet type
    // If not, silently update radio packet type
    if( PacketType != packetParams->PacketType )
    {
        SUBGRF_SetPacketType( packetParams->PacketType );
    }

    switch( packetParams->PacketType )
    {
    case PACKET_TYPE_GMSK:
    case PACKET_TYPE_GFSK:
        if( packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_IBM )
        {
            SUBGRF_SetCrcSeed( CRC_IBM_SEED );
            SUBGRF_SetCrcPolynomial( CRC_POLYNOMIAL_IBM );
            crcVal = RADIO_CRC_2_BYTES;
        }
        else if( packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_CCIT )
        {
            SUBGRF_SetCrcSeed( CRC_CCITT_SEED );
            SUBGRF_SetCrcPolynomial( CRC_POLYNOMIAL_CCITT );
            crcVal = RADIO_CRC_2_BYTES_INV;
        }
        else
        {
            crcVal = packetParams->Params.Gfsk.CrcLength;
        }
        n = 9;
        buf[0] = ( packetParams->Params.Gfsk.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.Gfsk.PreambleLength;
        buf[2] = packetParams->Params.Gfsk.PreambleMinDetect;
        buf[3] = ( packetParams->Params.Gfsk.SyncWordLength /*<< 3*/ ); // convert from byte to bit
        buf[4] = packetParams->Params.Gfsk.AddrComp;
        buf[5] = packetParams->Params.Gfsk.HeaderType;
        buf[6] = packetParams->Params.Gfsk.PayloadLength;
        buf[7] = crcVal;
        buf[8] = packetParams->Params.Gfsk.DcFree;
        break;
    case PACKET_TYPE_BPSK:
        n = 1;
        buf[0] = packetParams->Params.Bpsk.PayloadLength;
        break;
    case PACKET_TYPE_LORA:
        n = 6;
        buf[0] = ( packetParams->Params.LoRa.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.LoRa.PreambleLength;
        buf[2] = LoRaHeaderType = packetParams->Params.LoRa.HeaderType;
        buf[3] = packetParams->Params.LoRa.PayloadLength;
        buf[4] = packetParams->Params.LoRa.CrcMode;
        buf[5] = packetParams->Params.LoRa.InvertIQ;
        break;
    default:
    case PACKET_TYPE_NONE:
        return;
    }
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_PACKETPARAMS, buf, n );
}

void SUBGRF_SetCadParams( RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout )
{
    uint8_t buf[7];

    buf[0] = ( uint8_t )cadSymbolNum;
    buf[1] = cadDetPeak;
    buf[2] = cadDetMin;
    buf[3] = ( uint8_t )cadExitMode;
    buf[4] = ( uint8_t )( ( cadTimeout >> 16 ) & 0xFF );
    buf[5] = ( uint8_t )( ( cadTimeout >> 8 ) & 0xFF );
    buf[6] = ( uint8_t )( cadTimeout & 0xFF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_CADPARAMS, buf, 7 );
    OperatingMode = MODE_CAD;
}

void SUBGRF_SetBufferBaseAddress( uint8_t txBaseAddress, uint8_t rxBaseAddress )
{
    uint8_t buf[2];

    buf[0] = txBaseAddress;
    buf[1] = rxBaseAddress;
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_BUFFERBASEADDRESS, buf, 2 );
}

RadioStatus_t SUBGRF_GetStatus( void )
{
    uint8_t stat = 0;
    RadioStatus_t status = { .Value = 0 };

    HAL_SUBGHZ_ExecGetCmd( &hsubghz, RADIO_GET_STATUS, ( uint8_t * )&stat, 1 );
    status.Fields.CmdStatus = ( stat & ( 0x07 << 1 ) ) >> 1;
    status.Fields.ChipMode = ( stat & ( 0x07 << 4 ) ) >> 4;
    return status;
}

int8_t SUBGRF_GetRssiInst( void )
{
    uint8_t buf[1];
    int8_t rssi = 0;

    HAL_SUBGHZ_ExecGetCmd( &hsubghz, RADIO_GET_RSSIINST, buf, 1 );
    rssi = -buf[0] >> 1;
    return rssi;
}

void SUBGRF_GetRxBufferStatus( uint8_t *payloadLength, uint8_t *rxStartBufferPointer )
{
    uint8_t status[2];
    uint8_t data;

    HAL_SUBGHZ_ExecGetCmd( &hsubghz, RADIO_GET_RXBUFFERSTATUS, status, 2 );

    // In case of LORA fixed header, the payloadLength is obtained by reading
    // the register REG_LR_PAYLOADLENGTH
    if( ( SUBGRF_GetPacketType( ) == PACKET_TYPE_LORA ) && ( LoRaHeaderType == LORA_PACKET_FIXED_LENGTH ) )
    {
        HAL_SUBGHZ_ReadRegisters( &hsubghz, REG_LR_PAYLOADLENGTH, &data, 1 );
        *payloadLength = data;
    }
    else
    {
        *payloadLength = status[0];
    }
    *rxStartBufferPointer = status[1];
}

void SUBGRF_GetPacketStatus( PacketStatus_t *pktStatus )
{
    uint8_t status[3];

    HAL_SUBGHZ_ExecGetCmd( &hsubghz, RADIO_GET_PACKETSTATUS, status, 3 );

    pktStatus->packetType = SUBGRF_GetPacketType( );
    switch( pktStatus->packetType )
    {
        case PACKET_TYPE_GFSK:
            pktStatus->Params.Gfsk.RxStatus = status[0];
            pktStatus->Params.Gfsk.RssiSync = -status[1] >> 1;
            pktStatus->Params.Gfsk.RssiAvg = -status[2] >> 1;
            pktStatus->Params.Gfsk.FreqError = 0;
            break;

        case PACKET_TYPE_LORA:
            pktStatus->Params.LoRa.RssiPkt = -status[0] >> 1;
            ( status[1] < 128 ) ? ( pktStatus->Params.LoRa.SnrPkt = status[1] >> 2 ) : ( pktStatus->Params.LoRa.SnrPkt = ( ( status[1] - 256 ) >> 2 ) );
            pktStatus->Params.LoRa.SignalRssiPkt = -status[2] >> 1;
            pktStatus->Params.LoRa.FreqError = FrequencyError;
            break;

        default:
        case PACKET_TYPE_NONE:
            // In that specific case, we set everything in the pktStatus to zeros
            // and reset the packet type accordingly
            RADIO_MEMSET8( pktStatus, 0, sizeof( PacketStatus_t ) );
            pktStatus->packetType = PACKET_TYPE_NONE;
            break;
    }
}

RadioError_t SUBGRF_GetDeviceErrors( void )
{
    uint8_t err[] = { 0, 0 };
    RadioError_t error = { .Value = 0 };

    HAL_SUBGHZ_ExecGetCmd( &hsubghz, RADIO_GET_ERROR, ( uint8_t * )err, 2 );
    error.Fields.PaRamp     = ( err[0] & ( 1 << 0 ) ) >> 0;
    error.Fields.PllLock    = ( err[1] & ( 1 << 6 ) ) >> 6;
    error.Fields.XoscStart  = ( err[1] & ( 1 << 5 ) ) >> 5;
    error.Fields.ImgCalib   = ( err[1] & ( 1 << 4 ) ) >> 4;
    error.Fields.AdcCalib   = ( err[1] & ( 1 << 3 ) ) >> 3;
    error.Fields.PllCalib   = ( err[1] & ( 1 << 2 ) ) >> 2;
    error.Fields.Rc13mCalib = ( err[1] & ( 1 << 1 ) ) >> 1;
    error.Fields.Rc64kCalib = ( err[1] & ( 1 << 0 ) ) >> 0;
    return error;
}

void SUBGRF_ClearDeviceErrors( void )
{
    uint8_t buf[2] = { 0x00, 0x00 };
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_CLR_ERROR, buf, 2 );
}

void SUBGRF_ClearIrqStatus( uint16_t irq )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( ( uint16_t )irq >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( ( uint16_t )irq & 0x00FF );
    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_CLR_IRQSTATUS, buf, 2 );
}

void SUBGRF_WriteRegister( uint16_t addr, uint8_t data )
{
    HAL_SUBGHZ_WriteRegisters( &hsubghz, addr, (uint8_t*)&data, 1 );
}

uint8_t SUBGRF_ReadRegister( uint16_t addr )
{
    uint8_t data;
    HAL_SUBGHZ_ReadRegisters( &hsubghz, addr, &data, 1 );
    return data;
}

void SUBGRF_WriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    HAL_SUBGHZ_WriteRegisters( &hsubghz, address, buffer, size );
}

void SUBGRF_ReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    HAL_SUBGHZ_ReadRegisters( &hsubghz, address, buffer, size );
}

void SUBGRF_WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    HAL_SUBGHZ_WriteBuffer( &hsubghz, offset, buffer, size );
}

void SUBGRF_ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    HAL_SUBGHZ_ReadBuffer( &hsubghz, offset, buffer, size );
}

/* HAL_SUBGHz Callbacks definitions */ 
void HAL_SUBGHZ_TxCpltCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_TX_DONE );
}

void HAL_SUBGHZ_RxCpltCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_RX_DONE );
}

void HAL_SUBGHZ_CRCErrorCallback (SUBGHZ_HandleTypeDef *hsubghz)
{
   RadioOnDioIrqCb( IRQ_CRC_ERROR);
}

void HAL_SUBGHZ_CADStatusCallback(SUBGHZ_HandleTypeDef *hsubghz, HAL_SUBGHZ_CadStatusTypeDef cadstatus)
{
  switch (cadstatus)
  {
    case HAL_SUBGHZ_CAD_CLEAR:
    RadioOnDioIrqCb( IRQ_CAD_CLEAR);
    break;
    case HAL_SUBGHZ_CAD_DETECTED:
    RadioOnDioIrqCb( IRQ_CAD_DETECTED);
    break;
    default:
    break;
  }
}

void HAL_SUBGHZ_RxTxTimeoutCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_RX_TX_TIMEOUT);
}

void HAL_SUBGHZ_HeaderErrorCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_HEADER_ERROR);
}

void HAL_SUBGHZ_PreambleDetectedCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_PREAMBLE_DETECTED);
}

void HAL_SUBGHZ_SyncWordValidCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_SYNCWORD_VALID);
}

void HAL_SUBGHZ_HeaderValidCallback(SUBGHZ_HandleTypeDef *hsubghz)
{
  RadioOnDioIrqCb( IRQ_HEADER_VALID);
}

/*Workaround 1.0 WL */
#define RCC_CR_HSEBYP            (0x1UL << 18U)

#define RCC_HSE_DisableBypass()  CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP)

#define RCC_HSE_EnableBypass()   SET_BIT(RCC->CR, RCC_CR_HSEBYP)

static void TCXO_Workaround( RF_API_Tcxo_State_t state )
{
  /* Workaround  needed only for cut1.0*/
  uint16_t RevisionID =  LL_DBGMCU_GetRevisionID(); 

  if (1U ==RBI_IsTCXO() && (RevisionID==0x1000) )
  {
    switch (state)
    {
    case RF_API_STATE_TCXO_OFF:
        LL_RCC_HSE_Disable();
        
        LL_RCC_HSE_DisableTcxo();
               
        RCC_HSE_DisableBypass();
        
        UTIL_LPM_SetStopMode( (1 << CFG_LPM_TCXO_WA_Id) , UTIL_LPM_ENABLE);
        
        MW_LOG(TS_ON,VLEVEL_H,"TCXOFF\r\n");
      break;
      
    case RF_API_STATE_TCXO_ON:
        LL_RCC_HSE_EnableTcxo();
                
        RCC_HSE_EnableBypass();

        LL_RCC_HSE_Enable();
        
        UTIL_LPM_SetStopMode( (1 << CFG_LPM_TCXO_WA_Id) , UTIL_LPM_DISABLE);
        
        MW_LOG(TS_ON,VLEVEL_H,"TCXON\r\n");
        
        while(LL_RCC_HSE_IsReady() == 0)
        {
        }
      
      break;

    default:
      break;
    }
  }
}

static void RadioReset_Workaround( void )
{
  if ( LL_DBGMCU_GetRevisionID() ==0x1000 )
  {
    uint8_t reg=0;

    HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_SLEEP, &reg, 1 );

    RADIO_DELAY_MS( 2 );
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
