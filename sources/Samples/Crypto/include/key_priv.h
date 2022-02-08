/**
  * @file: key_priv.h
  * @brief: This file define the key container structure.
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
  * 1.0.0 : 2019/12/27[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup Crypto
 * @{
 */

#ifndef Crypto_KEY_PRIV_H_
#define Crypto_KEY_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "crypto.h"

/*!
 * @struct
 * @brief This structure hold a key used for en/de-cryption and authentication.
 */
typedef struct  {
	uint8_t key[KEY_SIZE]; //! The key of 32 byte
} key_s;

/*!
 * @var
 * @brief This table hold all keys used in the system
 */
extern KEY_STORE key_s _a_Key_[];

#ifdef __cplusplus
}
#endif
#endif /* Crypto_KEY_PRIV_H_ */

/*! @} */
/*! @} */
/*! @} */
