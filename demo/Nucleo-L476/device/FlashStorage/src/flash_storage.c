
/**
  * @file: flash_storage.c
  * @brief: // TODO This file ...
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
  * 1.0.0 : 2021/02/18[GBI]
  * Initial version
  *
  *
  */
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "flash_storage.h"

/*!
 * @brief This
 *
 * @param [in]
 *
 * @return
 */
uint8_t FlashStorage_StoreInit(struct storage_area_s* pStoreArea)
{
	const struct flash_store_s* pFlashArea = pStoreArea->pFlashArea;
	uint32_t dest_addr;
	dev_res_e eRet = DEV_FAILURE;
	uint8_t i;
	// check if destination is page and double word aligned
	if ( pFlashArea &&
			!( ((uint32_t)pFlashArea)%FLASH_PAGE_SIZE ) &&
			!( ((uint32_t)pFlashArea)%sizeof(uint64_t) ) )
	{
		// check that total size dosen't exceed the page size
		uint64_t sum = 0;
		for (i = 0 ; i < NB_STORE_PART; i++)
		{
			sum += pStoreArea->u32Size[i];
		}
		if (sum <= FLASH_PAGE_SIZE)
		{
			// Erase the dedicated flash page
			dest_addr = (uint32_t)( pFlashArea );

			uint8_t retry = 3;
			do {
				eRet = BSP_Flash_Erase(BSP_Flash_GetPage(dest_addr));
				if ( eRet == DEV_SUCCESS )
				{
					retry = 0;
					break;
				}
				retry--;
			} while (retry);
			pStoreArea->u16Status = 0x0000;
		}
	}
	return eRet;
}

/*!
 * @brief This
 *
 * @param [in]
 *
 * @return
 */
uint8_t FlashStorage_StoreFini(struct storage_area_s* pStoreArea)
{
	const struct flash_store_s* pFlashArea = pStoreArea->pFlashArea;
	struct flash_store_header_s flash_header;

	uint32_t dest_addr;
	uint32_t next_dest_addr;
	dev_res_e eRet = DEV_FAILURE;
	uint8_t i;

	// check if destination is page and double word aligned
	if ( pFlashArea &&
			!( ((uint32_t)pFlashArea)%FLASH_PAGE_SIZE ) &&
			!( ((uint32_t)pFlashArea)%sizeof(uint64_t) ) )
	{
		// Setup header
		flash_header.u16Status = 0x0000;
		flash_header.u16Crc = 0xBEEF;
		for (i = 0 ; i < NB_STORE_PART; i++)
		{
			flash_header.u32PartAddr[i] = pStoreArea->u32TgtAddr[i];
		}
		// Store header
		dest_addr = (uint32_t)( &(pFlashArea->sHeader) );
		next_dest_addr = BSP_Flash_Store(dest_addr,  (uint64_t*)(&flash_header), sizeof(struct flash_store_header_s));
		if (next_dest_addr != 0xFFFFFFFF)
		{
			eRet = DEV_SUCCESS;
		}
	}
	return eRet;
}

/*!
 * @brief This
 *
 * @param [in]
 *
 * @return
 */
uint8_t FlashStorage_StoreWrite(struct storage_area_s* pStoreArea)
{
	uint32_t next_dest_addr;
	dev_res_e eRet = DEV_FAILURE;
	uint8_t i;
	// check if destination is page and double word aligned
	next_dest_addr = (uint32_t)(&(pStoreArea->pFlashArea->aData[0]));
	if ( pStoreArea->pFlashArea && !( next_dest_addr%sizeof(uint64_t) ) )
	{
		eRet = DEV_SUCCESS;
		// Store first part
		for ( i = 0 ; i < NB_STORE_PART; i++)
		{
			pStoreArea->u32TgtAddr[i] = next_dest_addr;
			next_dest_addr = BSP_Flash_Store(pStoreArea->u32TgtAddr[i], (void*)pStoreArea->u32SrcAddr[i], pStoreArea->u32Size[i]);
			if (next_dest_addr == 0xFFFFFFFF)
			{
				eRet = DEV_FAILURE;
				break;
			}
			pStoreArea->u16Status++;
		}
	}
	return eRet;
}

/*!
 * @brief This
 *
 * @param [in]
 *
 * @return
 */
uint8_t FlashStorage_StoreRead(struct storage_area_s* pStoreArea)
{
	const struct flash_store_s* pFlashArea = pStoreArea->pFlashArea;
	dev_res_e eRet = DEV_FAILURE;
	// check if destination is page and double word aligned
	if ( pFlashArea )
	{
		memcpy( (void*)(pStoreArea->u32SrcAddr[0]), (void*)(pFlashArea->sHeader.u32PartAddr[0]), pStoreArea->u32Size[0]);
		memcpy( (void*)(pStoreArea->u32SrcAddr[1]), (void*)(pFlashArea->sHeader.u32PartAddr[1]), pStoreArea->u32Size[1]);
		memcpy( (void*)(pStoreArea->u32SrcAddr[2]), (void*)(pFlashArea->sHeader.u32PartAddr[2]), pStoreArea->u32Size[2]);
		eRet = DEV_SUCCESS;
	}
	return eRet;
}

#ifdef __cplusplus
}
#endif
