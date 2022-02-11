/**
  * @file: key.c
  * @brief: This file expose some helping function to deal with secured keys
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

#include "key_priv.h"
#include "utils_secure.h"

#ifdef HAS_KEY_STORE_IN_FLASH
uint8_t aFlashCache[FLASH_PAGE_SIZE];
#endif

static inline uint8_t _set_key_(uint8_t p_Key[KEY_SIZE], uint8_t u8_KeyId);

/*!
 *  @static
  * @fn uint8_t _set_key_(uint8_t p_Key[KEY_SIZE], uint8_t u8_KeyId)
  *
  * @brief This function set (write) a key at specified key id.
  *
  * @param [in] p_Key[KEY_SIZE] The given key to set.
  * @param [in] u8_KeyId The key id to use.
  * @retval CRYPTO_OK (1) if everything is fine
  * @retval CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  */
static inline uint8_t _set_key_(uint8_t p_Key[KEY_SIZE], uint8_t u8_KeyId)
{
    uint8_t u8_ret = CRYPTO_OK;
	// check key id
	if (u8_KeyId > KEY_MAX_NB) {
		u8_ret = CRYPTO_KID_UNK_ERR;
	}
#ifdef HAS_KEY_STORE_IN_FLASH

	key_s *pKey;
	uint32_t u32Page;
	// save flash page into the buffer
	memcpy(aFlashCache, _a_Key_, FLASH_PAGE_SIZE);
	// copy the new key
	pKey = &(((key_s*)aFlashCache)[u8_KeyId]);
	memcpy(pKey, p_Key, KEY_SIZE);
	// Erase the Flash corresponding page
	u32Page = ((uint32_t)_a_Key_ - 0x8000000)/FLASH_PAGE_SIZE;
	BSP_Flash_Erase(u32Page);
	// Write back into the Flash
	BSP_Flash_Write(
			(uint32_t)(&(_a_Key_[0])),
			(uint64_t*)(aFlashCache),
			FLASH_PAGE_SIZE/8
			);
#else
	//secure_memcpy(&(_a_Key_[u8_KeyId].key), p_Key, KEY_SIZE);
	memcpy(&(_a_Key_[u8_KeyId].key), p_Key, KEY_SIZE);
#endif

	return u8_ret;
}

/*!
  * @fn uint8_t Crypto_WriteKey(uint8_t p_Key[KEY_SIZE], uint8_t u8_KeyId)
  *
  * @brief This function set (write) a key at specified key id.
  *
  * @param [in] p_Key[KEY_SIZE] The given key to set.
  * @param [in] u8_KeyId The key id to use.
  * @retval CRYPTO_OK (1) if everything is fine
  * @retval CRYPTO_KID_UNK_ERR (2) id the key id is out of box
  */
uint8_t Crypto_WriteKey(uint8_t p_Key[KEY_SIZE], uint8_t u8_KeyId)
{
	return _set_key_(p_Key, u8_KeyId);
}

/*! @} */
/*! @} */
/*! @} */
