/**
  * @file bsp_flash.c
  * @brief This file implement bsp functions to access the internal flash device
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
  * @par 1.0.0 : 2021/02/19 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup flash
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_flash.h"
#include "platform.h"
#include <string.h>
#include <stm32l4xx_hal.h>

/**
  * @brief  Erase the given flash page area
  * @param  u32PageId Flash Page Id
  *
  * @retval DEV_SUCCESS if everything is fine (see @link dev_res_e::DEV_SUCCESS @endlink)
  * @retval DEV_FAILURE if failed (see @link dev_res_e::DEV_FAILURE @endlink)
  */
dev_res_e BSP_Flash_Erase(uint32_t u32PageId)
{
	dev_res_e eRet = DEV_FAILURE;
	FLASH_EraseInitTypeDef pEraseInit;
	uint32_t u32ErrCode;

	// TODO: enter critical section
	if (HAL_FLASH_Unlock() == HAL_OK)
	{
		pEraseInit.Banks = 0;
		pEraseInit.NbPages = 1;
		pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		pEraseInit.Page = u32PageId;
		if (HAL_FLASHEx_Erase(&pEraseInit, &u32ErrCode) == HAL_OK)
		{
			eRet = DEV_SUCCESS;
		}
		HAL_FLASH_Lock();
	}
	// TODO : exit critical section
	return eRet;
}

/*!
  * @brief  Erase the related flash memory area
  *
  * @param [in] u32Address Start address of area to erase
  * @param [in] u32NbBytes Area size to erase
  *
  * @retval DEV_SUCCESS if everything is fine (see @link dev_res_e::DEV_SUCCESS @endlink)
  * @retval DEV_FAILURE if failed (see @link dev_res_e::DEV_FAILURE @endlink)
  */
dev_res_e BSP_Flash_EraseArea(uint32_t u32Address, uint32_t u32NbBytes)
{
	dev_res_e eRet = DEV_SUCCESS;
	uint16_t page_id;
	uint16_t last_page;

	if ( (u32Address + u32NbBytes) % FLASH_PAGE_SIZE )
	{
		// out of page size
		eRet = DEV_FAILURE;
	}
	else
	{
		last_page = BSP_Flash_GetPage(u32Address + u32NbBytes);
		page_id = BSP_Flash_GetPage(u32Address);
		do
		{
			uint8_t retry = 3;
			do {
				eRet = BSP_Flash_Erase(page_id);
				if ( eRet == DEV_SUCCESS )
				{
					retry = 0;
					break;
				}
				retry--;
			} while (retry);

			if (eRet != DEV_SUCCESS)
			{
				break;
			}
			page_id++;
		} while(page_id < last_page);
	}
	return eRet;
}

/**
  * @brief  Write double-word aligned data
  * @param  u32Address  Flash Address
  * @param  pData       Pointer on data sto store
  * @param  u32NbDword  The number of double-word of data to store
  *
  * @retval DEV_SUCCESS if everything is fine (see @link dev_res_e::DEV_SUCCESS @endlink)
  * @retval DEV_FAILURE if failed (see @link dev_res_e::DEV_FAILURE @endlink)
  */
dev_res_e BSP_Flash_Write(uint32_t u32Address, uint64_t *pData, uint32_t u32NbDword)
{
	//uint32_t err_code;
	uint32_t i;
	int32_t u32TgtAdd;

	dev_res_e eRet = DEV_FAILURE;
	// TODO: enter critical section
	if (HAL_FLASH_Unlock() == HAL_OK)
	{
		eRet = DEV_SUCCESS;
		u32TgtAdd = u32Address;

		uint64_t data;
		for (i = 0; i <u32NbDword; i++)
		{
			// FIXME : bad!!! but it's seems required that pData is aligned on 8 bytes
			memcpy((void*)&data, (void*)&(pData[i]), 8);

			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32TgtAdd, data) != HAL_OK)
			{
				eRet = DEV_FAILURE;
				//err_code = HAL_FLASH_GetError();
				break;
			}
			u32TgtAdd += sizeof(uint64_t);
		}
		HAL_FLASH_Lock();
	}
	// TODO : exit critical section
	return eRet;

}

/**
  * @brief  Store the given data into Flash memory
  * @note : If the number of byte to write is not double-word aligned, the rest
  *         is filled with padding (0xFF).
  * @param  u32Address  Flash Address
  * @param  pData       Pointer on data to store
  * @param  u32NbBytes  The number of byte of data to store
  *
  * @retval the next 64 bits aligned flash address if everything is fine.
  * @retval 0xFFFFFFFF if failed.
  */
uint32_t BSP_Flash_Store(uint32_t u32Address, void* pData, uint32_t u32NbBytes)
{
	uint8_t *p;
	uint64_t last;
	uint32_t next_dest_addr;
	uint32_t nb_dbl_word;
	uint32_t nb_bytes_aligned;
	uint32_t remains_bytes;

	p = (uint8_t*)pData;
	next_dest_addr = u32Address;
	nb_dbl_word = (u32NbBytes/sizeof(uint64_t));
	nb_bytes_aligned = nb_dbl_word*sizeof(uint64_t);
	remains_bytes = ( u32NbBytes%sizeof(uint64_t) );
	// treat double-word aligned part
	if ( DEV_SUCCESS != BSP_Flash_Write(next_dest_addr, (uint64_t*)p, nb_dbl_word))
	{
		return 0xFFFFFFFF;
	}
	next_dest_addr += nb_bytes_aligned;
	// treat remains bytes and add padding
	if (remains_bytes)
	{
		p += nb_bytes_aligned;
		last = 0xFFFFFFFFFFFFFFFF;
		memcpy((void*)(&last), (void*)p, remains_bytes);
		nb_dbl_word = 1;
		if ( DEV_SUCCESS != BSP_Flash_Write(next_dest_addr, &last, nb_dbl_word))
		{
			return 0xFFFFFFFF;
		}
		next_dest_addr += sizeof(uint64_t);
	}
	return next_dest_addr;
}

/**
  * @brief  Obtains the page id from the given address
  * @param  u32Address Flash Address
  * @retval The Flash page id
  */
uint32_t BSP_Flash_GetPage(uint32_t u32Address)
{
	return (u32Address - FLASH_BASE) / FLASH_PAGE_SIZE;
}

#ifdef __cplusplus
}
#endif

/*! @} */
