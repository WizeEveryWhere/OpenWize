/**
  * @file: flash_storage.h
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
#ifndef _FLASH_STORAGE_H_
#define _FLASH_STORAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bsp.h"

#define FLASH_PAGE_SIZE 2048
#define NB_STORE_PART 3
struct flash_store_header_s
{
	uint16_t u16Status;
	uint16_t u16Crc;
	uint32_t u32PartAddr[NB_STORE_PART];
};

struct flash_store_s
{
	struct flash_store_header_s sHeader;
	uint8_t aData[FLASH_PAGE_SIZE - sizeof(struct flash_store_header_s)];
};

struct storage_area_s
{
	uint16_t u16Status;
	uint16_t u16Crc;
	uint32_t u32Size[NB_STORE_PART];
	uint32_t u32SrcAddr[NB_STORE_PART];
	uint32_t u32TgtAddr[NB_STORE_PART];
	const struct flash_store_s* pFlashArea;
};

uint8_t FlashStorage_StoreInit(struct storage_area_s* pStoreArea);
uint8_t FlashStorage_StoreFini(struct storage_area_s* pStoreArea);
uint8_t FlashStorage_StoreWrite(struct storage_area_s* pStoreArea);
uint8_t FlashStorage_StoreRead(struct storage_area_s* pStoreArea);

#ifdef __cplusplus
}
#endif
#endif /* _FLASH_STORAGE_H_ */
