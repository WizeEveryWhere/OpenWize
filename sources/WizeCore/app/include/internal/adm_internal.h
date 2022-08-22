/**
  * @file: adm_internal.h
  * @brief This file define the functions to treat the administration L7
  * 
  * @details
  *
  * @copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *
  * @par Revision history
  *
  * @par 1.0.0 : 2020/10/11[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_admin_layer
 * @{
 *
 */
#ifndef _ADM_INTERNAL_H_
#define _ADM_INTERNAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "proto.h"
#include "app_layer.h"

uint8_t AdmInt_PreCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_Unknown(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_ReadParam(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_WriteParam(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_WriteKey(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_Execping(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_Anndownload(net_msg_t *pReqMsg, net_msg_t *pRspMsg);

#ifdef __cplusplus
}
#endif
#endif /* _ADM_INTERNAL_H_ */

/*! @} */
