/**
  * @file: crypto.h
  * @brief: This file expose the crypto API for integrity and confidentiality.
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
  * 1.0.0 : 2020/12/31[GBI]
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

#ifndef Crypto_CRYPTO_H_
#define Crypto_CRYPTO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef HAS_KEY_STORE_IN_FLASH
#define KEY_STORE const
#else
#define KEY_STORE
#endif

/*!
 * @brief Returned code from Crypto functions
 */
typedef enum{
	CRYPTO_KO = 0,			 //!< Crypto function failed
	CRYPTO_OK = 1,			 //!< Crypto function success
	CRYPTO_KID_UNK_ERR = 2,  //!< The key id is unknown
	CRYPTO_INT_NULL_ERR = 4, //!< Error due to a NULL pointer
}crypto_code_e;

// Keys management

/*!
 * @def KEY_MAX_NB
 * @brief Define the maximum number of key
 */
#define KEY_MAX_NB 20

/*!
 * @def KEY_NONE_ID
 * @brief Define the key id for no encryption
 */
#define KEY_NONE_ID 0

/*!
 * @def KEY_ENC_MIN
 * @brief Define the first key id for the kenc key
 */
#define KEY_ENC_MIN 1

/*!
 * @def KEY_ENC_MAX
 * @brief Define the last key id for the kenc key
 */
#define KEY_ENC_MAX 14

/*!
 * @def KEY_CHG_ID
 * @brief Define the key id for the kchg key
 */
#define KEY_CHG_ID 15

/*!
 * @def KEY_MOB_ID
 * @brief Define the key id for the kmob key
 */
#define KEY_MOB_ID 16

/*!
 * @def KEY_MAC_ID
 * @brief Define the key id for the kmac key
 */
#define KEY_MAC_ID 17

/*!
 * @def KEY_LOG_ID
 * @brief Define the key id for the klog key
 */
#define KEY_LOG_ID 18

/*!
 * @def KEY_TOTAL_BYTE_SIZE
 * @brief Define the total number of byte for a key
 */
#define KEY_TOTAL_BYTE_SIZE 32

/*!
 * @def KEY_USED_BYTE_SIZE
 * @brief Define the number of used byte in a key
 */
#define KEY_USED_BYTE_SIZE 16

/*!
 * @def KEY_SIZE
 * @brief Define the key size (alias for @link KEY_TOTAL_BYTE_SIZE @endlink)
 */
#define KEY_SIZE KEY_TOTAL_BYTE_SIZE

/*!
 * @def CTR_SIZE
 * @brief Define the CTR (counter) size used by the AES algorithm (alias for
 * @link KEY_USED_BYTE_SIZE @endlink)
 */
#define CTR_SIZE KEY_USED_BYTE_SIZE

/*!
 * @def SHA256_SZ
 * @brief Define the size of sha256
 */
#define SHA256_SIZE 32

// Data confidentiality
uint8_t Crypto_Encrypt(uint8_t *p_Out, uint8_t *p_In, uint8_t u8_Sz,
				uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);

uint8_t Crypto_Decrypt(uint8_t *p_Out, uint8_t *p_In, uint8_t u8_Sz,
				uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);

// Data integrity
uint8_t Crypto_AES128_CMAC(uint8_t *p_Hash, uint8_t *p_Msg, uint8_t u8_Sz,
		uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);

uint8_t Crypto_SHA256(uint8_t p_Sha256[SHA256_SIZE], uint8_t *p_Data,
		uint32_t u32_Sz);

// Key write
uint8_t Crypto_WriteKey(uint8_t p_Key[KEY_SIZE], uint8_t u8_KeyId);

#ifdef __cplusplus
}
#endif
#endif /* Crypto_CRYPTO_H_ */

/*! @} */
/*! @} */
/*! @} */
