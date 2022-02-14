/**
  * @file: proto_api.h
  * @brief: This file define public function to deal with the Wize protocol
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
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
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2020/09/14[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup WizeCore
 * @{
 *
 */
#ifndef _PROTO_API_H_
#define _PROTO_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "proto_private.h"

uint8_t Wize_ProtoBuild(struct proto_ctx_s *pCtx, net_msg_t *pNetMsg);
uint8_t Wize_ProtoExtract(struct proto_ctx_s *pCtx, net_msg_t *pNetMsg);
void Wize_ProtoStats_RxUpdate(struct proto_ctx_s *pCtx, uint8_t u8ErrCode, uint8_t u8Rssi);
void Wize_ProtoStats_TxUpdate(struct proto_ctx_s *pCtx, uint8_t u8ErrCode, uint8_t u8Noise);
void Wize_ProtoStats_RxClear(struct proto_ctx_s *pCtx);
void Wize_ProtoStats_TxClear(struct proto_ctx_s *pCtx);
const char * Wize_Proto_GetStrErr(uint8_t eErr);

#ifdef __cplusplus
}
#endif
#endif /* _PROTO_API_H_ */

/*! @} */
