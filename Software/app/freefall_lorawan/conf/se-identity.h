/*!
 * \file      se-identity.h
 *
 * \brief     Secure Element identity and keys
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
 *              (C)2020 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 */
/**
  ******************************************************************************
  *
  *          Portions COPYRIGHT 2020 STMicroelectronics
  *
  * @file    se-identity_template.c
  * @author  MCD Application Team
  * @brief   Secure Element identity and keys
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOFT_SE_IDENTITY_H__
#define __SOFT_SE_IDENTITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_conf.h"

/*!
 ******************************************************************************
 ********************************** WARNING ***********************************
 ******************************************************************************
  The secure-element implementation supports both 1.0.x and 1.1.x LoRaWAN
  versions of the specification.
  Thus it has been decided to use the 1.1.x keys and EUI name definitions.
  The below table shows the names equivalence between versions:
               +---------------------+-------------------------+
               |       1.0.x         |          1.1.x          |
               +=====================+=========================+
               | LORAWAN_DEVICE_EUI  | LORAWAN_DEVICE_EUI      |
               +---------------------+-------------------------+
               | LORAWAN_APP_EUI     | LORAWAN_JOIN_EUI        |
               +---------------------+-------------------------+
               | LORAWAN_GEN_APP_KEY | LORAWAN_APP_KEY         |
               +---------------------+-------------------------+
               | LORAWAN_APP_KEY     | LORAWAN_NWK_KEY         |
               +---------------------+-------------------------+
               | LORAWAN_NWK_S_KEY   | LORAWAN_F_NWK_S_INT_KEY |
               +---------------------+-------------------------+
               | LORAWAN_NWK_S_KEY   | LORAWAN_S_NWK_S_INT_KEY |
               +---------------------+-------------------------+
               | LORAWAN_NWK_S_KEY   | LORAWAN_NWK_S_ENC_KEY   |
               +---------------------+-------------------------+
               | LORAWAN_APP_S_KEY   | LORAWAN_APP_S_KEY       |
               +---------------------+-------------------------+
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 */

// TODO: Improve readability of this file, see https://github.com/TheThingsIndustries/generic-node-se/issues/56

/*!
 * When set to 1 DevEui is LORAWAN_DEVICE_EUI
 * When set to 0 DevEui is automatically set with a value provided by MCU platform
 */
#define STATIC_DEVICE_EUI                                  1

/*!
 * end-device IEEE EUI (big endian)
 */
#define LORAWAN_DEVICE_EUI                                 { DEVEUI }

/*!
 * App/Join server IEEE EUI (big endian)
 */
#define LORAWAN_JOIN_EUI                                   { APPEUI }

/*!
 * When set to 1 DevAddr is LORAWAN_DEVICE_ADDRESS
 * When set to 0 DevAddr is automatically set with a value provided by a pseudo
 *      random generator seeded with a value provided by the MCU platform
 */
#define STATIC_DEVICE_ADDRESS                              0

/*!
 * Device address on the network (big endian)
 */
#define LORAWAN_DEVICE_ADDRESS                             ( uint32_t )0x00000000

/*!
 * Application root key
 */
#define LORAWAN_APP_KEY                                   { APPKEY}

/*!
 * Network root key
 */
#define LORAWAN_NWK_KEY                                   { APPKEY }

/*!
 * Forwarding Network session key
 */
#define LORAWAN_NWK_S_KEY                                  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }

/*!
 * Application session key
 */
#define LORAWAN_APP_S_KEY                                  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }

#if (USE_LRWAN_1_1_X_CRYPTO == 1)
#define SESSION_KEYS_LIST                                                                                           \
           {                                                                                                        \
            /*!                                                                                                     \
             * Join session integrity key (Dynamically updated)                                                     \
             * WARNING: NOT USED FOR 1.0.x DEVICES                                                                  \
             */                                                                                                     \
            .KeyID    = J_S_INT_KEY,                                                                                \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Join session encryption key (Dynamically updated)                                                    \
             * WARNING: NOT USED FOR 1.0.x DEVICES                                                                  \
             */                                                                                                     \
            .KeyID    = J_S_ENC_KEY,                                                                                \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Forwarding Network session integrity key                                                             \
             * WARNING: NWK_S_KEY FOR 1.0.x DEVICES                                                                 \
             */                                                                                                     \
            .KeyID    = F_NWK_S_INT_KEY,                                                                            \
            .KeyValue = LORAWAN_NWK_S_KEY,                                                              \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Serving Network session integrity key                                                                \
             * WARNING: NOT USED FOR 1.0.x DEVICES. MUST BE THE SAME AS \ref LORAWAN_F_NWK_S_INT_KEY                \
             */                                                                                                     \
            .KeyID    = S_NWK_S_INT_KEY,                                                                            \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Network session encryption key                                                                       \
             * WARNING: NOT USED FOR 1.0.x DEVICES. MUST BE THE SAME AS \ref LORAWAN_F_NWK_S_INT_KEY                \
             */                                                                                                     \
            .KeyID    = NWK_S_ENC_KEY,                                                                              \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Application session key                                                                              \
             */                                                                                                     \
            .KeyID    = APP_S_KEY,                                                                                  \
            .KeyValue = LORAWAN_APP_S_KEY,                                                              \
        },
#else /* USE_LRWAN_1_1_X_CRYPTO == 0 */
#define SESSION_KEYS_LIST                                                                                           \
        {                                                                                                           \
            /*!                                                                                                     \
             * Network session key                                                                                  \
             */                                                                                                     \
            .KeyID    = NWK_S_KEY,                                                                                  \
            .KeyValue = LORAWAN_NWK_S_KEY,                                                              \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Application session key                                                                              \
             */                                                                                                     \
            .KeyID    = APP_S_KEY,                                                                                  \
            .KeyValue = LORAWAN_APP_S_KEY,                                                              \
        },
#endif /* USE_LRWAN_1_1_X_CRYPTO */

#if (LORAMAC_MAX_MC_CTX == 1)
#define SESSION_MC_KEYS_LIST                                                                                        \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #0 root key (Dynamically updated)                                                    \
             */                                                                                                     \
            .KeyID    = MC_KEY_0,                                                                                   \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #0 application session key (Dynamically updated)                                     \
             */                                                                                                     \
            .KeyID    = MC_APP_S_KEY_0,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #0 network session key (Dynamically updated)                                         \
             */                                                                                                     \
            .KeyID    = MC_NWK_S_KEY_0,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },
#else /* LORAMAC_MAX_MC_CTX > 1 */
#define SESSION_MC_KEYS_LIST                                                                                        \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #0 root key (Dynamically updated)                                                    \
             */                                                                                                     \
            .KeyID    = MC_KEY_0,                                                                                   \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #0 application session key (Dynamically updated)                                     \
             */                                                                                                     \
            .KeyID    = MC_APP_S_KEY_0,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #0 network session key (Dynamically updated)                                         \
             */                                                                                                     \
            .KeyID    = MC_NWK_S_KEY_0,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #1 root key (Dynamically updated)                                                    \
             */                                                                                                     \
            .KeyID    = MC_KEY_1,                                                                                   \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #1 application session key (Dynamically updated)                                     \
             */                                                                                                     \
            .KeyID    = MC_APP_S_KEY_1,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #1 network session key (Dynamically updated)                                         \
             */                                                                                                     \
            .KeyID    = MC_NWK_S_KEY_1,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #2 root key (Dynamically updated)                                                    \
             */                                                                                                     \
            .KeyID    = MC_KEY_2,                                                                                   \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #2 application session key (Dynamically updated)                                     \
             */                                                                                                     \
            .KeyID    = MC_APP_S_KEY_2,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #2 network session key (Dynamically updated)                                         \
             */                                                                                                     \
            .KeyID    = MC_NWK_S_KEY_2,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #3 root key (Dynamically updated)                                                    \
             */                                                                                                     \
            .KeyID    = MC_KEY_3,                                                                                   \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #3 application session key (Dynamically updated)                                     \
             */                                                                                                     \
            .KeyID    = MC_APP_S_KEY_3,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast group #3 network session key (Dynamically updated)                                         \
             */                                                                                                     \
            .KeyID    = MC_NWK_S_KEY_3,                                                                             \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },
#endif /* LORAMAC_MAX_MC_CTX */

#define SOFT_SE_KEY_LIST                                                                                            \
    {                                                                                                               \
        {                                                                                                           \
            /*!                                                                                                     \
             * Application root key                                                                                 \
             * WARNING: FOR 1.0.x DEVICES IT IS THE \ref LORAWAN_GEN_APP_KEY                                        \
             */                                                                                                     \
            .KeyID    = APP_KEY,                                                                                    \
            .KeyValue = LORAWAN_APP_KEY,                                                                \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Network root key                                                                                     \
             * WARNING: FOR 1.0.x DEVICES IT IS THE \ref LORAWAN_APP_KEY                                            \
             */                                                                                                     \
            .KeyID    = NWK_KEY,                                                                                    \
            .KeyValue = LORAWAN_NWK_KEY,                                                                \
        },                                                                                                          \
        SESSION_KEYS_LIST                                                                                           \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast root key (Dynamically updated)                                                             \
             */                                                                                                     \
            .KeyID    = MC_ROOT_KEY,                                                                                \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        {                                                                                                           \
            /*!                                                                                                     \
             * Multicast key encryption key (Dynamically updated)                                                   \
             */                                                                                                     \
            .KeyID    = MC_KE_KEY,                                                                                  \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
        SESSION_MC_KEYS_LIST                                                                                        \
        {                                                                                                           \
            /*!                                                                                                     \
             * All zeros key. (ClassB usage)(constant)                                                              \
             */                                                                                                     \
            .KeyID    = SLOT_RAND_ZERO_KEY,                                                                         \
            .KeyValue = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                          0x00 },                                                                                   \
        },                                                                                                          \
    }

#ifdef __cplusplus
}
#endif

#endif  /*  __SOFT_SE_IDENTITY_H__ */
