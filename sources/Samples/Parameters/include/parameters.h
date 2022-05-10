/**
  * @file parameters.h
  * @brief This file define function to deal with parameters.
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
  * @par 1.0.0 : 2019/11/20 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup parameters
 * @{
 *
 */
#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "parameters_def.h"

void Param_Init(const uint8_t *p_Param);
uint8_t Param_IsValidId(uint8_t u8_Id);
uint8_t Param_GetSize(uint8_t u8_Id);
param_access_e Param_GetLocAccess(uint8_t u8_Id);
param_access_e Param_GetRemAccess(uint8_t u8_Id);
param_effective_e Param_GetEffect(uint8_t u8_Id);
param_ref_e Param_GetReferenced(uint8_t u8_Id);
uint32_t Param_GetAddOf(uint8_t u8_Id);
uint8_t Param_GetRestrId(uint8_t u8_Id);
const void* Param_GetRestrTableAdd(uint8_t u8_Id);
uint8_t Param_CheckConformity(uint8_t u8_Id, uint8_t* p_Data);

uint8_t Param_Access(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir);
uint8_t Param_LocalAccess(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir);
uint8_t Param_RemoteAccess(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir);

#ifdef __cplusplus
}
#endif
#endif /* _PARAMETERS_H_ */

/*! @} */
