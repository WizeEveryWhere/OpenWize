/*!
  * @file: confidentiality.c
  * @brief: This file expose functions to encrypt or decrypt a given message.
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
  * 1.0.0 : 2019/11/25[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Samples
 * @{
 *
 */
#ifdef __cplusplus
}
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "key_priv.h"
#include "utils_secure.h"
#include "tinycrypt/ctr_mode.h"

#if TC_AES_BLOCK_SIZE != CTR_SIZE
#error "Incompatible AES block size!!"
#endif

static uint8_t _crypt_(uint8_t *p_Out, uint8_t *p_In, uint8_t u8_Sz,
				uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);


/*!
  * @fn inline uint8_t Crypto_Encrypt(uint8_t *p_Out, uint8_t *p_In,
  * 				   uint8_t u8_Sz, uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
  * @brief Wrapper around _crypt_ function.
  *
  * @detail This function encrypt (with the AES128 in CTR mode) a given message.
  *
  * @param [in,out] p_Out Pointer on output buffer (Cipher text).
  * @param [in,out] p_In Pointer on input buffer (Plain text).
  * @param [in] u8_Sz Input buffer size
  * @param [in] p_Ctr Counter buffer. Warning : it will be altered by this function.
  * @param [in] u8_KeyId The key id to use for encrypt
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
inline uint8_t Crypto_Encrypt(uint8_t *p_Out, uint8_t *p_In, uint8_t u8_Sz,
				uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
{
	return _crypt_(p_Out, p_In, u8_Sz,	p_Ctr, u8_KeyId);
}

/*!
  * @fn inline uint8_t Crypto_Decrypt(uint8_t *p_Out, uint8_t *p_In,
  * 				   uint8_t u8_Sz, uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
  * @brief Wrapper around the _crypt_ function.
  *
  * @detail This function decrypt (with the AES128 in CTR mode) a given message.
  *
  * @param [in,out] p_Out Pointer on output buffer (Plain text).
  * @param [in,out] p_In Pointer on input buffer (Cipher text).
  * @param [in] u8_Sz Input buffer size
  * @param [in] p_Ctr Counter buffer. Warning : it will be altered by this function.
  * @param [in] u8_KeyId The key id to use for decrypt
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
inline uint8_t Crypto_Decrypt(uint8_t *p_Out, uint8_t *p_In, uint8_t u8_Sz,
				uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
{
	return _crypt_(p_Out, p_In, u8_Sz, p_Ctr, u8_KeyId);
}

/*!
  * @static
  * @brief This function en/de crypt with the AES128 in CTR mode.
  *
  * @param [in,out] p_Out Pointer on output buffer. Plain text for decrypt, cipher text for encrypt.
  * @param [in,out] p_In Pointer on input buffer. Cipher text for encrypt, plain text for decrypt.
  * @param [in] u8_Sz Input buffer size
  * @param [in] p_Ctr Counter buffer. Warning : it will be altered by this function.
  * @param [in] u8_KeyId The key id to use for encrypt or decrypt
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
static uint8_t _crypt_(uint8_t *p_Out, uint8_t *p_In, uint8_t u8_Sz,
				uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
{
	struct tc_aes_key_sched_struct s_sched;
	uint8_t u8_ret = CRYPTO_OK;
	// check key id
	if (u8_KeyId > KEY_MAX_NB) {
		u8_ret = CRYPTO_KID_UNK_ERR;
	}
	// check sanity
	if (p_Out == NULL || p_In == NULL || u8_Sz == 0 || p_Ctr == NULL) {
		u8_ret = CRYPTO_INT_NULL_ERR;
	}

	if (  u8_ret == CRYPTO_OK ) {
		if (u8_KeyId != 0) {
			u8_ret = tc_aes128_set_encrypt_key(&s_sched, _a_Key_[u8_KeyId].key);
			u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(tc_ctr_mode(
								   p_Out, u8_Sz, p_In, u8_Sz, p_Ctr, &s_sched));
			u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(CRYPTO_OK);
		}
		else {
			memcpy(p_Out, p_In, u8_Sz);
		}
	}
	return u8_ret;
}

#ifdef __cplusplus
}
#endif

/*! @} */
