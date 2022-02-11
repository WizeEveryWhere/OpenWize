/*!
  * @file: integrity.c
  * @brief: This file expose functions to compute AES128-CMAC footprint of a given message.
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
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup Crypto
 * @{
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <tinycrypt/cmac_mode.h>
#include <tinycrypt/constants.h>
#include <tinycrypt/aes.h>
#include <tinycrypt/sha256.h>
#include "key_priv.h"
#include "utils_secure.h"

static uint8_t _AES128_CMAC_(uint8_t *p_Hash, uint8_t *p_Msg, uint8_t u8_Sz,
		uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);
static uint8_t _SHA256_(uint8_t p_Sha256[SHA256_SIZE], uint8_t *p_Data, uint32_t u32_Sz);

/*!
  * @fn inline uint8_t Crypto_AES128_CMAC(uint8_t *p_Hash, uint8_t *p_Msg,
  *			           uint8_t u8_Sz, uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
  * @brief Wrapper around the _AES128_CMAC_ function.
  *
  * @detail This function compute the footprint (with the AES128 in CMAC mode) of a given message.
  *
  * @param [in,out] p_Hash Pointer on output buffer (footprint).
  * @param [in,out] p_Msg Pointer on input buffer (the message on which the footprint is computed)
  * @param [in] u8_Sz Input buffer size
  * @param [in] p_Ctr Counter buffer.
  * @param [in] u8_KeyId The key id to use for compute the footprint
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
inline uint8_t Crypto_AES128_CMAC(uint8_t *p_Hash, uint8_t *p_Msg, uint8_t u8_Sz,
		uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
{
	return _AES128_CMAC_(p_Hash, p_Msg, u8_Sz, p_Ctr, u8_KeyId);
}

/*!
  * @fn inline uint8_t Crypto_SHA256(uint8_t p_Digest[SHA256_SIZE], uint8_t *p_Data, uint8_t u8_Sz)
  * @brief Wrapper around the _SHA256_ function.
  *
  * @detail This function compute the SHA256 of given buffer..
  *
  * @param [in]  p_Sha256 Pointer on output buffer (sha256, 32 bytes).
  * @param [out] p_Data   Pointer on input buffer (the message on which the footprint is computed)
  * @param [in]  u32_Sz   Input buffer size
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
inline uint8_t Crypto_SHA256(uint8_t p_Sha256[SHA256_SIZE], uint8_t *p_Data, uint32_t u32_Sz)
{
	return _SHA256_(p_Sha256, p_Data, u32_Sz);
}

/*!
  * @static
  * @brief This function compute the footprint with the AES128 in CMAC mode.
  *
  * @param [in,out] p_Hash Pointer on output buffer (footprint).
  * @param [in,out] p_Msg Pointer on input buffer (the message on which the footprint is computed)
  * @param [in] u8_Sz Input buffer size
  * @param [in] p_Ctr Counter buffer.
  * @param [in] u8_KeyId The key id to use for compute the footprint
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
static uint8_t _AES128_CMAC_(uint8_t *p_Hash, uint8_t *p_Msg, uint8_t u8_Sz,
		uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId)
{

	struct tc_cmac_struct state;
	struct tc_aes_key_sched_struct s_sched;
    uint8_t buff[CTR_SIZE + u8_Sz];
    uint8_t msg_sz = sizeof(buff);
    uint8_t u8_ret = CRYPTO_OK;
	// check key id
	if (u8_KeyId > KEY_MAX_NB) {
		u8_ret = CRYPTO_KID_UNK_ERR;
	}
	// check sanity
	if (p_Hash == NULL || p_Msg == NULL || u8_Sz == 0 || p_Ctr == NULL) {
		u8_ret = CRYPTO_INT_NULL_ERR;
	}

    if (u8_ret == CRYPTO_OK) {
        memcpy(buff, p_Ctr, CTR_SIZE);
        memcpy(&(buff[CTR_SIZE]), p_Msg , u8_Sz);
    	u8_ret = tc_cmac_setup(&state, _a_Key_[u8_KeyId].key, &s_sched);
    	u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(tc_cmac_init(&state));
    	u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(tc_cmac_update(&state, buff, msg_sz));
    	u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(tc_cmac_final(p_Hash, &state));
    	u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(CRYPTO_OK);
    }
	return u8_ret;
}

/*!
  * @static
  * @brief This function compute the SHA256 of given buffer.
  *
  * @param [in]  p_Sha256 Pointer on output buffer (sha256, 32 bytes).
  * @param [out] p_Data   Pointer on input buffer (the message on which the footprint is computed)
  * @param [in]  u32_Sz    Input buffer size
  * @retval return crypto_code_e::CRYPTO_OK (1) if everything is fine
  *         return crypto_code_e::CRYPTO_KO (0) if something goes wrong
  *         return crypto_code_e::CRYPTO_INT_NULL_ERR (4) if one of the given pointer is NULL
  */
static uint8_t _SHA256_(uint8_t p_Sha256[SHA256_SIZE], uint8_t *p_Data, uint32_t u32_Sz)
{
	struct tc_sha256_state_struct state;
	uint8_t u8_ret = CRYPTO_OK;
	if ( p_Data == NULL || p_Sha256 == NULL || u32_Sz == 0 )	{
		u8_ret = CRYPTO_INT_NULL_ERR;
	}

    if (u8_ret == CRYPTO_OK)
    {
		u8_ret = tc_sha256_init(&state);
		u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(tc_sha256_update(&state, p_Data, u32_Sz));
		u8_ret = (u8_ret != TC_CRYPTO_SUCCESS)?(CRYPTO_KO):(tc_sha256_final(p_Sha256, &state));
    }
	return u8_ret;
}

/*! @} */
/*! @} */
/*! @} */
