/**
  ******************************************************************************
  * @file    kms_platf_objects_interface_template.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Key Management Services (KMS)
  *          module platform objects management configuration interface
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef KMS_PLATF_OBJECTS_INTERFACE_H
#define KMS_PLATF_OBJECTS_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pkcs11.h"
#include "pkcs11t.h"
#include "Commissioning.h"

/* KMS exported Object handles for tKMS test application */
#define KMS_ZERO_KEY_OBJECT_HANDLE                 ((CK_OBJECT_HANDLE)0)
#define KMS_APP_KEY_OBJECT_HANDLE                  ((CK_OBJECT_HANDLE)1)
#define KMS_NWK_KEY_OBJECT_HANDLE                  ((CK_OBJECT_HANDLE)2)
#define KMS_GEN_APP_KEY_OBJECT_HANDLE              ((CK_OBJECT_HANDLE)3)
/* only for OTAA==0 */
#define KMS_F_NWK_S_INT_KEY_OBJECT_HANDLE          ((CK_OBJECT_HANDLE)4)
#define KMS_S_NWK_S_INT_KEY_OBJECT_HANDLE          ((CK_OBJECT_HANDLE)5)
#define KMS_NWK_S_ENC_KEY_OBJECT_HANDLE            ((CK_OBJECT_HANDLE)6)
#define KMS_APP_S_KEY_OBJECT_HANDLE                ((CK_OBJECT_HANDLE)7)
#define LAST_KMS_KEY_OBJECT_HANDLE                 ((CK_OBJECT_HANDLE)8)


#ifdef __cplusplus
}
#endif

#endif /* KMS_PLATF_OBJECTS_INTERFACE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

