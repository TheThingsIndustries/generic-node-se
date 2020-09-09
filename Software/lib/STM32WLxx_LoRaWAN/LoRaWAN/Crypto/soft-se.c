/*!
 * \file      soft-se.c
 *
 * \brief     Secure Element software implementation
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
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 */
/**
  ******************************************************************************
  *
  *          Portions COPYRIGHT 2020 STMicroelectronics
  *
  * @file    soft-se.c
  * @author  MCD Application Team
  * @brief   Secure Element software implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "secure-element.h"

#include "lorawan_conf.h"  /* LORAWAN_KMS */
#include "mw_log_conf.h"   /* needed for MW_LOG */
#include "radio.h"         /* needed for Random */ 

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
#include "cmac.h"
#else /* LORAWAN_KMS == 1 */
#include "kms_if.h"
#endif /* LORAWAN_KMS */

/* Private constants ---------------------------------------------------------*/

//#define KMS_DEBUG  
/*!
 * Number of supported crypto keys
 */
#define NUM_OF_KEYS      24UL

/*!
 * Crypto keys size in bytes
 */
#define KEY_SIZE         16UL

/*!
 * Size of JoinReqType is field for integrity check
 * \remark required for 1.1.x support
 */
#define JOIN_REQ_TYPE_SIZE 1

/*!
 * Size of DevNonce is field for integrity check
 * \remark required for 1.1.x support
 */
#define DEV_NONCE_SIZE 2

/*!
 * MIC computation offset
 * \remark required for 1.1.x support
 */
#define CRYPTO_MIC_COMPUTATION_OFFSET ( JOIN_REQ_TYPE_SIZE + LORAMAC_JOIN_EUI_FIELD_SIZE + DEV_NONCE_SIZE + LORAMAC_MHDR_FIELD_SIZE )

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
#else /* LORAWAN_KMS == 1 */
#define DERIVED_OBJECT_HANDLE_RESET_VAL      0x0UL
#define PAYLOAD_MAX_SIZE     270UL  /* 270 PHYPayload: 1+(22+1+242)+4 */
#endif /* LORAWAN_KMS */


/* Private Types ---------------------------------------------------------*/

/*!
 * Identifier value pair type for Keys
 */
typedef struct sKey
{
    /*
     * Key identifier
     */
    KeyIdentifier_t KeyID;

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    /*
     * Key value
     */
    uint8_t KeyValue[KEY_SIZE];
#else /* LORAWAN_KMS == 1 */
    /*
     * Key object index in the above list
     */
    CK_OBJECT_HANDLE   Object_Index;
#endif
} Key_t;

/*
 * Secure Element Non Volatile Context structure
 */
typedef struct sSecureElementNvCtx
{
    /*
     * DevEUI storage
     */
    uint8_t DevEui[SE_EUI_SIZE];
    /*
     * Join EUI storage
     */
    uint8_t JoinEui[SE_EUI_SIZE];
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    /*
     * AES computation context variable
     */
    aes_context AesContext;
    /*
     * CMAC computation context variable
     */
    AES_CMAC_CTX AesCmacCtx[1];
#endif
    /*
     * Key List
     */
    Key_t KeyList[NUM_OF_KEYS];
}SecureElementNvCtx_t;


/* Private variables ---------------------------------------------------------*/

/*
 * Module context
 */
static SecureElementNvCtx_t SeNvmCtx;

static SecureElementNvmEvent SeNvmCtxChanged;

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
#else /* LORAWAN_KMS == 1 */

/*
 * Module context
 */

static CK_ULONG DeriveKey_template_class = CKO_SECRET_KEY;
static CK_ULONG DeriveKey_template_destroyable = CK_TRUE;
static CK_ULONG DeriveKey_template_encrypt = CK_TRUE;
static CK_ULONG DeriveKey_template_decrypt = CK_TRUE;
static CK_ULONG DeriveKey_template_extract = CK_FALSE;
static CK_ATTRIBUTE DeriveKey_template[] =
{
  {CKA_CLASS, (CK_VOID_PTR) &DeriveKey_template_class,       sizeof(CK_ULONG)},
  {CKA_DESTROYABLE, (CK_VOID_PTR) &DeriveKey_template_destroyable, sizeof(CK_ULONG)},
  {CKA_ENCRYPT, (CK_VOID_PTR) &DeriveKey_template_encrypt,     sizeof(CK_ULONG)},
  {CKA_DECRYPT, (CK_VOID_PTR) &DeriveKey_template_decrypt,     sizeof(CK_ULONG)},
  {CKA_EXTRACTABLE, (CK_VOID_PTR) &DeriveKey_template_extract,     sizeof(CK_ULONG)}
};

#ifdef KMS_DEBUG
static CK_ATTRIBUTE GetKey_template[] = {{CKA_VALUE, (CK_VOID_PTR) &DeriveKey_template_class, KEY_SIZE}};
#endif

/*
 * Intermediate buffer used for two reasons: 
 * - align to 32 bits and
 * - for Cmac combine InitVector + input buff
 */
static uint8_t input_align_combined_buf[PAYLOAD_MAX_SIZE+KEY_SIZE] ALIGN(4);
static uint8_t output_align[PAYLOAD_MAX_SIZE] ALIGN(4);
static uint8_t tag[KEY_SIZE] ALIGN(4) = {0};


#endif /* LORAWAN_KMS */

/* Private functions prototypes ---------------------------------------------------*/
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
static SecureElementStatus_t GetKeyByID( KeyIdentifier_t keyID, Key_t** keyItem );
#else /* LORAWAN_KMS == 1 */
static SecureElementStatus_t GetKeyIndexByID( KeyIdentifier_t keyID, CK_OBJECT_HANDLE *keyItem );
#endif /* LORAWAN_KMS */

static SecureElementStatus_t ComputeCmac( uint8_t *micBxBuffer, uint8_t *buffer, uint16_t size, KeyIdentifier_t keyID, uint32_t* cmac );
static void DummyCB( void );
#ifdef KMS_DEBUG
static void dbg_print_buffer(uint8_t *pbuff, uint32_t length);
#endif

/* Private functions ---------------------------------------------------------*/
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
/*
 * Gets key item from key list.
 *
 *  cmac = aes128_cmac(keyID, B0 | msg)
 *
 * \param[IN]  keyID          - Key identifier
 * \param[OUT] keyItem        - Key item reference
 * \retval                    - Status of the operation
 */
static SecureElementStatus_t GetKeyByID( KeyIdentifier_t keyID, Key_t** keyItem )
{
    for( uint8_t i = 0; i < NUM_OF_KEYS; i++ )
    {
        if( SeNvmCtx.KeyList[i].KeyID == keyID )
        {
            *keyItem = &( SeNvmCtx.KeyList[i] );
            return SECURE_ELEMENT_SUCCESS;
        }
    }
    return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
}

#else /* LORAWAN_KMS == 1 */

/*
 * Gets key Index from key list in KMS table
 *
 * \param[IN]  keyID          - Key identifier
 * \param[OUT] keyIndex       - Key item reference
 * \retval                    - Status of the operation
 */
static SecureElementStatus_t GetKeyIndexByID( KeyIdentifier_t keyID, CK_OBJECT_HANDLE *keyIndex )
{
    for( uint8_t i = 0; i < NUM_OF_KEYS; i++ )
    {
        if( SeNvmCtx.KeyList[i].KeyID == keyID )
        {
            *keyIndex = SeNvmCtx.KeyList[i].Object_Index;
            return SECURE_ELEMENT_SUCCESS;
        }
    }
    return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
}

#endif /* LORAWAN_KMS */


/*
 * Computes a CMAC of a message using provided initial Bx block
 *
 *  cmac = aes128_cmac(keyID, blocks[i].Buffer)
 *
 * \param[IN]  micBxBuffer    - Buffer containing the initial Bx block
 * \param[IN]  buffer         - Data buffer
 * \param[IN]  size           - Data buffer size
 * \param[IN]  keyID          - Key identifier to determine the AES key to be used
 * \param[OUT] cmac           - Computed cmac
 * \retval                    - Status of the operation
 */
static SecureElementStatus_t ComputeCmac( uint8_t* micBxBuffer, uint8_t* buffer, uint16_t size, KeyIdentifier_t keyID,
                                          uint32_t* cmac )
{
    if( ( buffer == NULL ) || ( cmac == NULL ) )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    uint8_t Cmac[16];

    AES_CMAC_Init( SeNvmCtx.AesCmacCtx );

    Key_t* keyItem;
    SecureElementStatus_t retval = GetKeyByID( keyID, &keyItem );

    if( retval == SECURE_ELEMENT_SUCCESS )
    {
        AES_CMAC_SetKey( SeNvmCtx.AesCmacCtx, keyItem->KeyValue );

        if( micBxBuffer != NULL )
        {
            AES_CMAC_Update( SeNvmCtx.AesCmacCtx, micBxBuffer, 16 );
        }

        AES_CMAC_Update( SeNvmCtx.AesCmacCtx, buffer, size );

        AES_CMAC_Final( Cmac, SeNvmCtx.AesCmacCtx );

        // Bring into the required format
        *cmac = ( uint32_t )( ( uint32_t ) Cmac[3] << 24 | ( uint32_t ) Cmac[2] << 16 | ( uint32_t ) Cmac[1] << 8 | ( uint32_t ) Cmac[0] );
    }

#ifdef KMS_DEBUG
        MW_LOG(TS_OFF, VLEVEL_L, "Cmac: %d %d %d %d, MIC: %d \r\n", Cmac[3], Cmac[2], Cmac[1], Cmac[0], *cmac);
#endif

#else /* LORAWAN_KMS == 1 */

  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  uint32_t tag_lenth = 0;
  SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
  CK_OBJECT_HANDLE object_handle ;

  /* AES CMAC Authentication variables */
  CK_MECHANISM aes_cmac_mechanism = { CKM_AES_CMAC, (CK_VOID_PTR)NULL, 0 };

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Configure session to Authentication message in AES CMAC with settings included into the mechanism */
  retval = GetKeyIndexByID(keyID, &object_handle);
  if( retval != SECURE_ELEMENT_SUCCESS )
  {
    return retval;
  }
  if (rv == CKR_OK)
  {
    rv = C_SignInit(session, &aes_cmac_mechanism, object_handle);
  }

  /* Encrypt clear message */
  if (rv == CKR_OK)
  {
    // work around : need to double-check if possible to use micBxBuffer as IV for Sign
    if( micBxBuffer != NULL )
    {
      memcpy1( ( uint8_t* ) &input_align_combined_buf, ( uint8_t* ) micBxBuffer, KEY_SIZE );  
      memcpy1( ( uint8_t* ) &input_align_combined_buf+KEY_SIZE, ( uint8_t* ) buffer, size );
    }
  }

  if (rv == CKR_OK)
  {
    if( micBxBuffer != NULL )
    {
      rv = C_Sign(session, (CK_BYTE_PTR)input_align_combined_buf, size+KEY_SIZE, &tag[0], (CK_ULONG_PTR)&tag_lenth);
    }
    else
    {
      rv = C_Sign(session, (CK_BYTE_PTR)buffer, size, &tag[0], (CK_ULONG_PTR)&tag_lenth);
    }
  }

  /* Close session with KMS */
  (void)C_CloseSession(session);

  /* combine to a 32bit authentication word (MIC) */
  *cmac = ( uint32_t )( ( uint32_t ) tag[3] << 24 | ( uint32_t ) tag[2] << 16 | ( uint32_t ) tag[1] << 8 | ( uint32_t ) tag[0] );

#ifdef KMS_DEBUG
  MW_LOG(TS_OFF, VLEVEL_H, "Cmac: %d %d %d %d, MIC: %d \r\n", tag[3], tag[2], tag[1], tag[0], *cmac);
#endif

  if (rv == CKR_OK)
  {
    retval = SECURE_ELEMENT_SUCCESS;
  }

#endif /* LORAWAN_KMS */

    return retval;
}

#ifdef KMS_DEBUG
static void dbg_print_buffer(uint8_t *pbuff, uint32_t length)
{ 
   uint8_t i;
   MW_LOG(TS_OFF, VLEVEL_H, "[0x");
   for(i=0; i<length; i++)
     MW_LOG(TS_OFF, VLEVEL_H, "%02X" , pbuff[i]);
   MW_LOG(TS_OFF, VLEVEL_H, "]\r\n");
}
#endif

/*
 * Dummy callback in case if the user provides NULL function pointer
 */
static void DummyCB( void )
{
    return;
}


/* Exported functions ---------------------------------------------------------*/

/*
 * API functions
 */


SecureElementStatus_t SecureElementInit( SecureElementNvmEvent seNvmCtxChanged )
{
    uint8_t itr = 0;

    // Initialize with defaults
    SeNvmCtx.KeyList[itr++].KeyID = APP_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = GEN_APP_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = NWK_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = J_S_INT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = J_S_ENC_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = F_NWK_S_INT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = S_NWK_S_INT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = NWK_S_ENC_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = APP_S_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = MC_ROOT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KE_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_0;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_0;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_0;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_1;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_1;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_1;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_2;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_2;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_2;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_3;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_3;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_3;
    SeNvmCtx.KeyList[itr].KeyID = SLOT_RAND_ZERO_KEY;

    memset1( SeNvmCtx.DevEui, 0, SE_EUI_SIZE );
    memset1( SeNvmCtx.JoinEui, 0, SE_EUI_SIZE );

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 

    uint8_t zeroKey[16] = { 0 };
    // Set standard keys
    memcpy1( SeNvmCtx.KeyList[itr].KeyValue, zeroKey, KEY_SIZE );

#else /* LORAWAN_KMS == 1 */
  CK_RV rv;

  /* Initialize tKMS for subsequent usage */
  rv = C_Initialize(NULL);
  if (rv != CKR_OK)
  {
    MW_LOG(TS_OFF, VLEVEL_L, "tKMS initialization failed\r\n");
    while(1){}  //ErrorHandler()
  }
  else
  {
    MW_LOG(TS_OFF, VLEVEL_L, "tKMS initialization done\r\n");
  }

#endif /* LORAWAN_KMS */

  // Assign callback
  if( seNvmCtxChanged != 0 )
  {
      SeNvmCtxChanged = seNvmCtxChanged;
  }
  else
  {
      SeNvmCtxChanged = DummyCB;
  }

  return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementDeleteDerivedKeys(uint8_t* kms_key_label)
{
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
#else /* LORAWAN_KMS == 1 */
  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Work Around: waiting for bugzilla Ticket 80990  */
  /* Be sure there are no remaining derived key in the NVM */
  /* The problem is that today there is no a Close session procedure */
  /* on the other hand this could erase keys generated by the application */
  /* so be very carefull with this ... */
  if (rv == CKR_OK)
  {
    rv = C_DestroyObject(session, 50);
    rv = C_DestroyObject(session, 51);
    rv = C_DestroyObject(session, 52);
    rv = C_DestroyObject(session, 53);
    rv = C_DestroyObject(session, 54);
    rv = C_DestroyObject(session, 55);
    rv = C_DestroyObject(session, 56);
    rv = C_DestroyObject(session, 57);
  }
  else
  {
    return SECURE_ELEMENT_ERROR;
  }

  /* Close sessions */
  if (session > 0)
  {
    (void)C_CloseSession(session);
  }
#endif /* LORAWAN_KMS == 1 */
  return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementRestoreNvmCtx( void* seNvmCtx )
{
    // Restore nvm context
    if( seNvmCtx != 0 )
    {
        memcpy1( ( uint8_t* ) &SeNvmCtx, ( uint8_t* ) seNvmCtx, sizeof( SeNvmCtx ) );
        return SECURE_ELEMENT_SUCCESS;
    }
    else
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
}

void* SecureElementGetNvmCtx( size_t* seNvmCtxSize )
{
    *seNvmCtxSize = sizeof( SeNvmCtx );
    return &SeNvmCtx;
}

SecureElementStatus_t SecureElementSetKey( KeyIdentifier_t keyID, uint8_t* key )
{
    if( key == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    for( uint8_t i = 0; i < NUM_OF_KEYS; i++ )
    {
        if( SeNvmCtx.KeyList[i].KeyID == keyID )
        {
            if( ( keyID == MC_KEY_0 ) || ( keyID == MC_KEY_1 ) || ( keyID == MC_KEY_2 ) || ( keyID == MC_KEY_3 ) )
            {  
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
                // Decrypt the key if its a Mckey
                SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
                uint8_t decryptedKey[16] = { 0 };

                retval = SecureElementAesEncrypt( key, 16, MC_KE_KEY, decryptedKey );

                memcpy1( SeNvmCtx.KeyList[i].KeyValue, decryptedKey, KEY_SIZE );
                SeNvmCtxChanged( );

                return retval;
#else /* LORAWAN_KMS == 1 */
                /* To be studied */
                /* do not return erro because it compromises applcation execution */
                return SECURE_ELEMENT_SUCCESS;
#endif                
            }
            else
            {
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
                memcpy1( SeNvmCtx.KeyList[i].KeyValue, key, KEY_SIZE );
                SeNvmCtxChanged( );
                return SECURE_ELEMENT_SUCCESS;
#else /* LORAWAN_KMS == 1 */
                /* Currently DOES NOTHING */
                /* Indexes are already stored at init or when deriving the key */
                /* to be studied */
                return SECURE_ELEMENT_SUCCESS;
#endif 
            }
        }
    }
    return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
}

SecureElementStatus_t SecureElementSetObjHandler(KeyIdentifier_t keyID, uint32_t keyIndex)
{
#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    return SECURE_ELEMENT_ERROR;
#else /* LORAWAN_KMS == 1 */
    for( uint8_t i = 0; i < NUM_OF_KEYS; i++ )
    {
        if( SeNvmCtx.KeyList[i].KeyID == keyID )
        {
          SeNvmCtx.KeyList[i].Object_Index = (CK_OBJECT_HANDLE) keyIndex;
          SeNvmCtxChanged( );
          return SECURE_ELEMENT_SUCCESS;
        }
    }
    return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
#endif /* LORAWAN_KMS */
}

SecureElementStatus_t SecureElementComputeAesCmac( uint8_t* micBxBuffer, uint8_t* buffer, uint16_t size,
                                                   KeyIdentifier_t keyID, uint32_t* cmac )
{
    if( keyID >= LORAMAC_CRYPTO_MULTICAST_KEYS )
    {
        // Never accept multicast key identifier for cmac computation
        return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
    }

    return ComputeCmac( micBxBuffer, buffer, size, keyID, cmac );
}

SecureElementStatus_t SecureElementVerifyAesCmac( uint8_t* buffer, uint16_t size, uint32_t expectedCmac,
                                                  KeyIdentifier_t keyID )
{
    if( buffer == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
    uint32_t compCmac = 0;
    retval = ComputeCmac( NULL, buffer, size, keyID, &compCmac );
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
        return retval;
    }

    if( expectedCmac != compCmac )
    {
        retval = SECURE_ELEMENT_FAIL_CMAC;
    }

    return retval;
}

SecureElementStatus_t SecureElementAesEncrypt( uint8_t* buffer, uint16_t size, KeyIdentifier_t keyID,
                                               uint8_t* encBuffer )
{
    if( buffer == NULL || encBuffer == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    // Check if the size is divisible by 16,
    if( ( size % 16 ) != 0 )
    {
        return SECURE_ELEMENT_ERROR_BUF_SIZE;
    }

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 

    memset1( SeNvmCtx.AesContext.ksch, '\0', 240 );

    Key_t* pItem;
    SecureElementStatus_t retval = GetKeyByID( keyID, &pItem );

    if( retval == SECURE_ELEMENT_SUCCESS )
    {
        aes_set_key( pItem->KeyValue, 16, &SeNvmCtx.AesContext );

        uint8_t block = 0;

        while( size != 0 )
        {
            aes_encrypt( &buffer[block], &encBuffer[block], &SeNvmCtx.AesContext );
            block = block + 16;
            size = size - 16;
        }
    }
    
#else /* LORAWAN_KMS == 1 */

  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  uint32_t encrypted_length = 0;
  SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
  CK_OBJECT_HANDLE object_handle;
  uint8_t dummy_tag[KEY_SIZE] = {0};
  uint32_t dummy_tag_lenth = 0;

  CK_MECHANISM aes_ecb_mechanism = { CKM_AES_ECB, (CK_VOID_PTR *) NULL, 0 };

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Configure session to encrypt message in AES ECB with settings included into the mechanism */
  
  retval = GetKeyIndexByID(keyID, &object_handle);
  if( retval != SECURE_ELEMENT_SUCCESS )
  {
    return retval;
  }
  if (rv == CKR_OK)
  {
    rv = C_EncryptInit(session, &aes_ecb_mechanism, object_handle);
  }

  /* Encrypt clear message */
  if (rv == CKR_OK)
  {
    memcpy1( input_align_combined_buf, buffer, size);
    encrypted_length = sizeof(output_align);
    rv = C_EncryptUpdate(session, (CK_BYTE_PTR)input_align_combined_buf, size,
                         output_align, (CK_ULONG_PTR)&encrypted_length);
    memcpy1(encBuffer, output_align, size);
  }

  /* In this case C_EncryptFinal is just called to Free the Alloc mem */
  if (rv == CKR_OK)
  {
    dummy_tag_lenth = sizeof(tag);
    rv = C_EncryptFinal(session, &dummy_tag[0], (CK_ULONG_PTR)&dummy_tag_lenth);
  }

  /* Close session with KMS */
  (void)C_CloseSession(session);

  if (rv == CKR_OK)
  {
    retval = SECURE_ELEMENT_SUCCESS;
  }

#endif /* LORAWAN_KMS */

#ifdef KMS_DEBUG
   MW_LOG(TS_OFF, VLEVEL_H, "encBuffer: " );
   dbg_print_buffer(encBuffer, KEY_SIZE);
#endif
  return retval;

}

SecureElementStatus_t SecureElementDeriveAndStoreKey( Version_t version, uint8_t* input, KeyIdentifier_t rootKeyID,
                                                      KeyIdentifier_t targetKeyID )
{
    if( input == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    // In case of MC_KE_KEY, prevent other keys than NwkKey or AppKey for LoRaWAN 1.1 or later
    if( targetKeyID == MC_KE_KEY )
    {
        if( ( ( rootKeyID == APP_KEY ) && ( version.Fields.Minor == 0 ) ) || ( rootKeyID == NWK_KEY ) )
        {
            return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
        }
    }

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0)) 
    SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
    uint8_t key[16] = { 0 };

    // Derive key
    retval = SecureElementAesEncrypt( input, 16, rootKeyID, key );
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
        return retval;
    }

    // Store key
    retval = SecureElementSetKey( targetKeyID, key );
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
        return retval;
    }

#ifdef KMS_DEBUG
   MW_LOG(TS_OFF, VLEVEL_H, "Derived key: " );
   dbg_print_buffer(key, KEY_SIZE);
#endif

    return SECURE_ELEMENT_SUCCESS;

#else /* LORAWAN_KMS == 1 */

  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  /* Key derivation */
  CK_MECHANISM      mech = {CKM_AES_ECB_ENCRYPT_DATA, input, KEY_SIZE};
  CK_OBJECT_HANDLE  derivedKeyHdle;
  CK_OBJECT_HANDLE  rootkey_object_handle;
  SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
#ifdef KMS_DEBUG
  uint8_t derivedKey[KEY_SIZE] = {0};
#endif //KMS_DEBUG

    // Derive key
    retval = GetKeyIndexByID(rootKeyID, &rootkey_object_handle);
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
      return retval;
    }

#ifdef KMS_DEBUG
    MW_LOG(TS_OFF, VLEVEL_H, "SecureElementDeriveAndStoreKey  \r\n" );
    MW_LOG(TS_OFF, VLEVEL_H, "root key ID : 0x%02X \r\n",  rootKeyID);
    MW_LOG(TS_OFF, VLEVEL_H, "root key object handler : 0x%02X \r\n",  rootkey_object_handle);
    MW_LOG(TS_OFF, VLEVEL_H, "input vector: " );
    dbg_print_buffer(input, KEY_SIZE);
#endif //KMS_DEBUG

    /* Open session with KMS */
    rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

    /* Derive key with pass phrase */
    if (rv == CKR_OK)
    {
      rv = C_DeriveKey(session, &(mech), rootkey_object_handle,
                     &DeriveKey_template[0], sizeof(DeriveKey_template) / sizeof(CK_ATTRIBUTE), &derivedKeyHdle);
    }

    /* Store Derived Index in table */
    retval = SecureElementSetObjHandler( targetKeyID, derivedKeyHdle);
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
      return retval;
    }

#ifdef KMS_DEBUG
    /* Get derived key to display */
    /* possible only because DeriveKey_template DeriveKey_template_extract = TRUE */
    /* after debug phase DeriveKey_template_extract shall be FALSE */
    if (DeriveKey_template_extract == CK_TRUE)
    {
      if (rv == CKR_OK)
      {
        GetKey_template[0].pValue = derivedKey;
        rv = C_GetAttributeValue(session, derivedKeyHdle, &(GetKey_template[0]), sizeof(GetKey_template) /
                             sizeof(CK_ATTRIBUTE));
      } 

      /* print log */
      MW_LOG(TS_OFF, VLEVEL_H, "Derived key: " );
      dbg_print_buffer(derivedKey, KEY_SIZE);
    }
#endif  //KMS_DEBUG

    /* Close session with KMS */
    (void)C_CloseSession(session);

    if (rv == CKR_OK)
    {
      return SECURE_ELEMENT_SUCCESS;
    }
    else
    {
      return SECURE_ELEMENT_ERROR;
    }

#endif /* LORAWAN_KMS */
}

SecureElementStatus_t SecureElementProcessJoinAccept( JoinReqIdentifier_t joinReqType, uint8_t* joinEui,
                                                      uint16_t devNonce, uint8_t* encJoinAccept,
                                                      uint8_t encJoinAcceptSize, uint8_t* decJoinAccept,
                                                      uint8_t* versionMinor )
{
    if( ( encJoinAccept == NULL ) || ( decJoinAccept == NULL ) || ( versionMinor == NULL ) )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    // Check that frame size isn't bigger than a JoinAccept with CFList size
    if( encJoinAcceptSize > LORAMAC_JOIN_ACCEPT_FRAME_MAX_SIZE )
    {
        return SECURE_ELEMENT_ERROR_BUF_SIZE;
    }

    // Determine decryption key
    KeyIdentifier_t encKeyID = NWK_KEY;

    if( joinReqType != JOIN_REQ )
    {
        encKeyID = J_S_ENC_KEY;
    }

    memcpy1( decJoinAccept, encJoinAccept, encJoinAcceptSize );

    // Decrypt JoinAccept, skip MHDR
    if( SecureElementAesEncrypt( encJoinAccept + LORAMAC_MHDR_FIELD_SIZE, encJoinAcceptSize - LORAMAC_MHDR_FIELD_SIZE,
                                 encKeyID, decJoinAccept + LORAMAC_MHDR_FIELD_SIZE ) != SECURE_ELEMENT_SUCCESS )
    {
        return SECURE_ELEMENT_FAIL_ENCRYPT;
    }

    *versionMinor = ( ( decJoinAccept[11] & 0x80 ) == 0x80 ) ? 1 : 0;

    uint32_t mic = 0;

    mic = ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE] << 0 );
    mic |= ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE + 1] << 8 );
    mic |= ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE + 2] << 16 );
    mic |= ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE + 3] << 24 );

    //  - Header buffer to be used for MIC computation
    //        - LoRaWAN 1.0.x : micHeader = [MHDR(1)]
    //        - LoRaWAN 1.1.x : micHeader = [JoinReqType(1), JoinEUI(8), DevNonce(2), MHDR(1)]

    // Verify mic
    if( *versionMinor == 0 )
    {
        // For LoRaWAN 1.0.x
        //   cmac = aes128_cmac(NwkKey, MHDR |  JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList |
        //   CFListType)
        if( SecureElementVerifyAesCmac( decJoinAccept, ( encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE ), mic, NWK_KEY ) !=
            SECURE_ELEMENT_SUCCESS )
        {
            return SECURE_ELEMENT_FAIL_CMAC;
        }
    }
#if( USE_LRWAN_1_1_X_CRYPTO == 1 )
    else if( *versionMinor == 1 )
    {
        uint8_t  micHeader11[JOIN_ACCEPT_MIC_COMPUTATION_OFFSET] = { 0 };
        uint16_t bufItr                                          = 0;

        micHeader11[bufItr++] = ( uint8_t ) joinReqType;

        memcpyr( micHeader11 + bufItr, joinEui, LORAMAC_JOIN_EUI_FIELD_SIZE );
        bufItr += LORAMAC_JOIN_EUI_FIELD_SIZE;

        micHeader11[bufItr++] = devNonce & 0xFF;
        micHeader11[bufItr++] = ( devNonce >> 8 ) & 0xFF;

        // For LoRaWAN 1.1.x and later:
        //   cmac = aes128_cmac(JSIntKey, JoinReqType | JoinEUI | DevNonce | MHDR | JoinNonce | NetID | DevAddr |
        //   DLSettings | RxDelay | CFList | CFListType)
        // Prepare the msg for integrity check (adding JoinReqType, JoinEUI and DevNonce)
        uint8_t localBuffer[LORAMAC_JOIN_ACCEPT_FRAME_MAX_SIZE + JOIN_ACCEPT_MIC_COMPUTATION_OFFSET] = { 0 };

        memcpy1( localBuffer, micHeader11, JOIN_ACCEPT_MIC_COMPUTATION_OFFSET );
        memcpy1( localBuffer + JOIN_ACCEPT_MIC_COMPUTATION_OFFSET - 1, decJoinAccept, encJoinAcceptSize );

        if( SecureElementVerifyAesCmac( localBuffer,
                                        encJoinAcceptSize + JOIN_ACCEPT_MIC_COMPUTATION_OFFSET -
                                            LORAMAC_MHDR_FIELD_SIZE - LORAMAC_MIC_FIELD_SIZE,
                                        mic, J_S_INT_KEY ) != SECURE_ELEMENT_SUCCESS )
        {
            return SECURE_ELEMENT_FAIL_CMAC;
        }
    }
#endif
    else
    {
        return SECURE_ELEMENT_ERROR_INVALID_LORAWAM_SPEC_VERSION;
    }

    return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementRandomNumber( uint32_t* randomNum )
{
    if( randomNum == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    *randomNum = Radio.Random( );

    return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementSetDevEui( uint8_t* devEui )
{
    if( devEui == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
    memcpy1( SeNvmCtx.DevEui, devEui, SE_EUI_SIZE );
    SeNvmCtxChanged( );
    return SECURE_ELEMENT_SUCCESS;
}

uint8_t* SecureElementGetDevEui( void )
{
    return SeNvmCtx.DevEui;
}

SecureElementStatus_t SecureElementSetJoinEui( uint8_t* joinEui )
{
    if( joinEui == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
    memcpy1( SeNvmCtx.JoinEui, joinEui, SE_EUI_SIZE );
    SeNvmCtxChanged( );
    return SECURE_ELEMENT_SUCCESS;
}

uint8_t* SecureElementGetJoinEui( void )
{
    return SeNvmCtx.JoinEui;
}

#if 0

/* This version is not currently working (only available for RSA but not for CMAC). 
   It will be activated when available. Current W.A. implemented above */

SecureElementStatus_t SecureElementVerifyAesCmac( uint8_t* buffer, uint16_t size, uint32_t expectedCmac, KeyIdentifier_t keyID )
{
  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  CK_OBJECT_HANDLE object_handle;
  SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
  uint32_t compCmac = 0;
  uint8_t expected_tag[4] = {0};


  if( buffer == NULL )
  {
    return SECURE_ELEMENT_ERROR_NPE;
  }

  // Check if the size is divisible by 16,
  if( ( size % 16 ) != 0 )
  {
    return SECURE_ELEMENT_ERROR_BUF_SIZE;
  }

  expected_tag[0] = expectedCmac & 0xFFFF;
  expected_tag[1] = (expectedCmac>>8) & 0xFFFF;
  expected_tag[2] = (expectedCmac>>16) & 0xFFFF;
  expected_tag[3] = (expectedCmac>>24) & 0xFFFF;   

  /* AES CMAC Authentication variables */
  CK_MECHANISM aes_cmac_mechanism = { CKM_AES_CMAC, (CK_VOID_PTR)NULL, 0 };

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Configure session to Verify the message in AES CMAC with settings included into the mechanism */
  retval = GetKeyIndexByID(keyID, &object_handle);
  if( retval != SECURE_ELEMENT_SUCCESS )
  {
    return retval;
  }
  if (rv == CKR_OK)
  {
    rv = C_VerifyInit(session, &aes_cmac_mechanism, object_handle);
  }

  /* Verify the message */
  if (rv == CKR_OK)
  {
    memcpy1( input_align_combined_buf, buffer, size);
    rv = C_Verify(session, (CK_BYTE_PTR)input_align_combined_buf, size, (CK_BYTE_PTR)expected_tag, 4);
  }

  (void)C_CloseSession(session);

  if (rv == CKR_OK)
  { 
    retval = SECURE_ELEMENT_SUCCESS;
  }

  return retval;
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
