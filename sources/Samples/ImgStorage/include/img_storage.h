/**
  * @file: img_storage..h
  * @brief: This file define function api to deal with Software image storage.
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
  * 1.0.0 : 2020/10/11[GBI]
  * Initial version
  *
  *
  */
#ifndef _IMG_STORAGE_H_
#define _IMG_STORAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/*!
 * @brief This enum define pending image entry
 */
typedef enum {
	PENDING_SETUP  = 0x00,
	PENDING_NONE   = 0x01,
	PENDING_LOCAL  = 0x11,
	PENDING_REMOTE = 0x21,
} img_pend_e;

typedef uint8_t (*pfWriteFlash_t)(uint32_t u32Addr, uint64_t *pData, uint32_t u32NbDoubleWord);
typedef uint8_t (*pfEraseFlash_t)(uint32_t u32Addr, uint32_t u32Size);

void ImgStore_StoreBlock(uint16_t u16_BlkId, uint8_t *p_Blk);
uint8_t ImgStore_IsComplete(void);
uint8_t ImgStore_Verify(uint8_t *pImgHash, uint8_t u8DigestSz);
uint8_t ImgStore_GetBitmapLine(uint16_t u16_Id);
uint16_t ImgStore_GetMaxBlockNb(void);
img_pend_e ImgStore_GetPending(void);
void ImgStore_SetPending(img_pend_e ePend);
int8_t ImgStore_Init(uint16_t u16NbExpectedBlk);
int8_t ImgStore_Setup(uint32_t u32ImgAdd, pfWriteFlash_t pfWrite, pfEraseFlash_t pfErase);


#ifdef __cplusplus
}
#endif
#endif /* _IMG_STORAGE_H_ */
