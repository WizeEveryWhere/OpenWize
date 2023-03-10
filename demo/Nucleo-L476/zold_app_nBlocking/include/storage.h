/**
  * @file storage.h
  * @brief This file define the storage functions
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
  * @par 1.0.0 : 2021/02/07 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup app
 * @{
 */
#ifndef _STORAGE_H_
#define _STORAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void Storage_Init(uint8_t bForce);
void Storage_SetDefault(void);

#ifdef __cplusplus
}
#endif
#endif /* _STORAGE_H_ */

/*! @} */
