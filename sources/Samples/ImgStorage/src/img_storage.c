
/**
  * @file: img_storage.c
  * @brief: This file implement functions to deal with Software image storage.
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

/*!
 * @ingroup Samples
 * @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "img_storage_private.h"
#include "crypto.h"

//#include "bsp_flash.h"

#include <assert.h>
#include <string.h>

/******************************************************************************/
#define PENDING_LINE_EMPTY_MSK 0x4000
#define PENDING_LINE_FULL_MSK 0x8000

uint8_t _get_bitmap_line(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id);
uint8_t _get_bitmap_bit(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id);
void _set_bitmap(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id);
void _clr_bitmap(struct img_mgr_ctx_s *pCtx);
void _clr_pending(struct img_mgr_ctx_s *pCtx);

struct img_mgr_ctx_s sImgMgrCtx;

/******************************************************************************/
/*!
  * @brief  Save the given software block and set its corresponding bit "on"
  * into the bitmap.
  *
  * @param [in] u16_BlkId The Id of the given block.
  * @param [in] *p_Blk    Pointer on the software block.
  *
  */
void ImgStore_StoreBlock(uint16_t u16_BlkId, uint8_t *p_Blk)
{
	uint32_t i;
	uint32_t u32Address;
	uint32_t u32Size;

	uint64_t *pData;
	uint8_t *pPend;
	uint16_t u16Idx;
    uint16_t u16PendBlkId;

    uint8_t u8NbPendAtStart;
	uint8_t u8NbPendAtEnd;

	uint8_t line;

	if ( (p_Blk == NULL) || (u16_BlkId > BLOCK_NB) )
	{
		// out of range
		return;
	}
    // check if block is already in
    if (_get_bitmap_bit(&sImgMgrCtx, u16_BlkId))
    {
    	// block is already download, so pass
    	return;
    }
    else
    { 	// block is not yet download
    	// get the pending line index
        u16Idx = (u16_BlkId >> 2)%(PENDING_LINE_NB -1);

    	// check if the pending line is available
        if ( sImgMgrCtx.sPendCtrl[u16Idx].EMPTY )
    	{
    		// the line is no more available
    	    sImgMgrCtx.sPendCtrl[u16Idx].EMPTY = 0;
        	// set pending block id (the first id in the pend line)
        	sImgMgrCtx.sPendCtrl[u16Idx].ID = u16_BlkId - u16_BlkId%4;
    	}

        // check if required pending line is already use by this block
        if ( (sImgMgrCtx.sPendCtrl[u16Idx].ID == ( u16_BlkId - u16_BlkId%4)) )
    	{
    		// Set the current block destination base address
    		//u32Address = sImgMgrCtx.u32Addr + ( ( (u16_BlkId * BLOCK_SZ) >> 3) << 3 ) ;
        	u32Address = sImgMgrCtx.u32Addr + (u16_BlkId * BLOCK_SZ);
    		/******************************************************************/
    		// deal with unaligned part(s) of the current block (pending bytes)

    		// Get pending line id
    		u16PendBlkId = u16_BlkId%4;
    		// Get the number of pending byte at Start end at End of the current block
    	    if (u16PendBlkId == 0)
    	    {
    	    	u8NbPendAtStart = 0;
    	    	u8NbPendAtEnd   = 2;
    	    }
    	    else if (u16PendBlkId == 1)
    	    {
    	    	u8NbPendAtStart = 6;
    	    	u8NbPendAtEnd   = 4;
    	    }
    	    else if (u16PendBlkId == 2)
    	    {
    	    	u8NbPendAtStart = 4;
    	    	u8NbPendAtEnd   = 6;
    	    }
    	    else if (u16PendBlkId == 3)
    	    {
    	    	u8NbPendAtStart = 2;
    	    	u8NbPendAtEnd   = 0;
    	    }
    	    else
    	    {// this should never happened
    	    	u8NbPendAtStart = 0;
    	    	u8NbPendAtEnd   = 0;
    	    }
        	// Store unaligned part(s) of the current block
    	    pPend = sImgMgrCtx.sPendBuff[u16Idx].aPend;
    	    pPend += u16PendBlkId << 3;

    	    for (i=0; i < u8NbPendAtEnd; i++)
    	    {
    	    	pPend[i] = p_Blk[ BLOCK_SZ - u8NbPendAtEnd +i];
    	    }

    	    pPend -= u8NbPendAtStart;
    	    for (i=0; i < u8NbPendAtStart; i++)
    	    {
    	    	pPend[i] = p_Blk[i];
    	    }

    	    pPend = sImgMgrCtx.sPendBuff[u16Idx].aPend;
    	    /******************************************************************/
    	    // deal with bytes already aligned on double-word
    		if (sImgMgrCtx.write)
    		{
    			// Get the start of pBlock
    			pData = (uint64_t*)&(p_Blk[u8NbPendAtStart]);
        		// Get nb double-word aligned to write
        		u32Size = (BLOCK_SZ - u8NbPendAtStart - u8NbPendAtEnd) >> 3;
        		// Get the offset to start address and write to flash
    			sImgMgrCtx.write(u32Address + u8NbPendAtStart, pData, u32Size);
    		}
			// set in bitmap the block id
			_set_bitmap(&sImgMgrCtx, u16_BlkId);


			/**************************************************************/
			// deal with pending double-word

			line = _get_bitmap_line(&sImgMgrCtx, u16_BlkId);
			line = ( (u16_BlkId%8) >> 2 )?(line):(line >> 4);
			line &= 0x0F;

			if ( ( ( line << u16PendBlkId ) & 0b1100 ) == 0b1100  ) // block after
			{
				pData = &( ((uint64_t*)pPend)[u16PendBlkId] );
				if ( sImgMgrCtx.write )
				{
					// Get the offset to start address and write to flash
					sImgMgrCtx.write(u32Address + BLOCK_SZ - u8NbPendAtEnd, pData, 1);
				}
				sImgMgrCtx.u32NbBlk++;
			}
			if ( ( ( line >> (3 - u16PendBlkId) ) & 0b0011 ) == 0b0011 ) // block before
			{
				pData = &( ((uint64_t*)pPend)[u16PendBlkId - 1] );
				if ( sImgMgrCtx.write )
				{
					// Get the offset to start address and write to flash
					sImgMgrCtx.write(u32Address - 8 + u8NbPendAtStart, pData, 1);
				}
				sImgMgrCtx.u32NbBlk++;
			}
			// Special case for the last block that is not "multiple of 4"
			if (sImgMgrCtx.u32NbExpectedBlk %4)
			{
				if ( u16_BlkId == (sImgMgrCtx.u32NbExpectedBlk -1) )
				{
					// Note that at this point :
					// - Double-word align data have already been written
					// - If exist, inter-block unaligned data have already been written
					// So, the only unaligned remaining part is at the end of the block


					pPend += u16PendBlkId << 3;
					// Add padding
					for ( i = u8NbPendAtEnd; i < 8; i++)
					{
						pPend[i] = 0xFF;
					}

					pData = (uint64_t*)pPend;
					if ( sImgMgrCtx.write )
					{
						// Get the offset to start address and write to flash
						sImgMgrCtx.write(u32Address + BLOCK_SZ - u8NbPendAtEnd, pData, 1);
					}
					sImgMgrCtx.u32NbBlk++;
				}
			}
			/**************************************************************/
			// if line is full, so release the buffer
			if ( ( line  & 0x0F ) == 0x0F )
			{
				sImgMgrCtx.sPendCtrl[u16Idx].ID = 0;
				sImgMgrCtx.sPendCtrl[u16Idx].EMPTY = 1;
				// Last block in pending line must be taken into account
				sImgMgrCtx.u32NbBlk++;
			}
			/**************************************************************/
    	}
    	// else, no space left, drop it
    }
}

/*!
  * @brief  Check if the given software image is complete
  *
  * @retval 0 Not complete
  * @retval 1 Complete
  */
uint8_t ImgStore_IsComplete(void)
{
	return (sImgMgrCtx.u32NbBlk == sImgMgrCtx.u32NbExpectedBlk);
}

/*!
  * @brief  Verify the sw image integrity (sha256).
  *
  * @param [in] pImgHash   Pointer on expected image sha256
  * @param [in] u8DigestSz Size of expected image sha256
  *
  * @retval 0 Success
  * @retval 1 Failed
  */
uint8_t ImgStore_Verify(uint8_t *pImgHash, uint8_t u8DigestSz)
{
	uint8_t pSha256[SHA256_SIZE];
	uint32_t u32_Sz = sImgMgrCtx.u32NbExpectedBlk*BLOCK_SZ;

	if ( Crypto_SHA256(pSha256, (uint8_t*)(sImgMgrCtx.u32Addr), u32_Sz) == CRYPTO_OK )
	{
		if ( pImgHash && (memcmp( pImgHash, pSha256, u8DigestSz) == 0) )
		{
			return 0;
		}
	}
	return 1;
}

/*!
  * @fn uint8_t ImgStore_GetBitmapLine(uint16_t u16_Id)
  * @brief Get the line value (from bitmap) of the given block Id.
  *
  * @param [in] u16_Id The Id of the block.
  * @retval The 8 bits bitmap line
  */
uint8_t ImgStore_GetBitmapLine(uint16_t u16_Id)
{
    return _get_bitmap_line(&sImgMgrCtx, (u16_Id+1) );
}

/*!
  * @fn uint16_t ImgStore_GetMaxBlockNb(void)
  * @brief Get the maximum admissible block number
  *
  * @retval The maximum block number
  */
uint16_t ImgStore_GetMaxBlockNb(void)
{
	return (uint16_t)sImgMgrCtx.u32Size;
}
/*!
  * @fn uint8_t ImgStore_GetPending(void)
  * @brief Get the pending flag.
  *
  * @retval return img_pend_e::PENDING_SETUP
  *         return img_pend_e::PENDING_NONE
  *         return img_pend_e::PENDING_LOCAL
  *         return img_pend_e::PENDING_REMOTE
  */
inline img_pend_e ImgStore_GetPending(void)
{
    return sImgMgrCtx.eImgPend;
}

/*!
  * @fn uint8_t ImgStore_SetPending(void)
  * @brief Set the pending flag.
  *
  * @param [in] ePend The pending flag to set
  *
  * @retval None
  */
inline void ImgStore_SetPending(img_pend_e ePend)
{
    sImgMgrCtx.eImgPend = ePend;
}

/*!
  * @brief  Initialize the image manager context.
  *
  * @param [in] u16NbExpectedBlk The number block for expected new sw image
  *
  * @retval  0 Success
  * @retval  1 Failed (at least one of given parameters is out of range)
  * @retval -1 Fatal (unable to erase the flash memory area)
  */
int8_t ImgStore_Init(uint16_t u16NbExpectedBlk)
{
	if (u16NbExpectedBlk > BLOCK_NB || ( sImgMgrCtx.eImgPend != PENDING_NONE) )
	{
		// the given expected number of block exceed the reserved area
		return 1;
	}
	// erase the memory area
	if ( sImgMgrCtx.erase ) {
		if ( sImgMgrCtx.erase(sImgMgrCtx.u32Addr, sImgMgrCtx.u32Size) )
		{
			// unexpected error while erasing the memory area
			return -1;
		}
	}
	// Clear all structure (bitmap, ...)
	sImgMgrCtx.u32NbBlk = 0;
	// erase the bitmap
	_clr_bitmap(&sImgMgrCtx);
	// erase the pending
	_clr_pending(&sImgMgrCtx);
	sImgMgrCtx.u32NbExpectedBlk = (uint32_t)u16NbExpectedBlk;
	return 0;
}

/*!
  * @brief  Initialize the image manager context.
  *
  * @param [in] u32ImgAdd        Software Image address (64 bits aligned)
  *
  * @retval  0 Success
  * @retval  1 Failed (at least one of given parameters is out of range)
  */
int8_t ImgStore_Setup(uint32_t u32ImgAdd, pfWriteFlash_t pfWrite, pfEraseFlash_t pfErase)
{
	if (u32ImgAdd & 0x7 || pfErase == NULL || pfWrite == NULL)
	{
		// the given image address is not aligned
		sImgMgrCtx.u32Addr = 0xFFFFFFFF;
		sImgMgrCtx.u32Size = 0;
		sImgMgrCtx.eImgPend = PENDING_SETUP;
		sImgMgrCtx.erase = NULL;
		sImgMgrCtx.write = NULL;
		return 1;
	}
	sImgMgrCtx.u32Addr = u32ImgAdd;
	sImgMgrCtx.u32Size = IMG_MAX_SZ; //IMG_NB_PAGES;
	sImgMgrCtx.eImgPend = PENDING_NONE;
	sImgMgrCtx.erase = pfErase; //&BSP_Flash_EraseArea;
	sImgMgrCtx.write = pfWrite; //&BSP_Flash_Write;
	return 0;
}

/******************************************************************************/

/*!
  * @static
  * @brief Get the line value (from bitmap) of the given block Id.
  *
  * @param [in] pCtx Pointer on image manager context
  * @param [in] u16_Id The Id of the block.
  *
  * @return the line value.
  */
uint8_t _get_bitmap_line(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id)
{
    return (pCtx->aBitMap[u16_Id/8]);
}

/*!
  * @static
  * @brief Get the bit value (from bitmap) of the given block Id.
  *
  * @param [in] pCtx Pointer on image manager context
  * @param [in] u16_Id The Id of the block.
  *
  * @retval 0 this block is not yet present.
  * @retval 1 this block is already present.
  */
uint8_t _get_bitmap_bit(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id)
{
    uint8_t u8_BitMask;
    u8_BitMask = 0b10000000 >> (u16_Id %8);
	// if 0 : we don't have yet get this block
	// if 1 : we have already download this block
	return (u8_BitMask & _get_bitmap_line(pCtx, u16_Id));
}

/*!
  * @static
  * @brief Set "on" the bit corresponding to the given block Id into the bitmap.
  *
  * @param [in] pCtx Pointer on image manager context
  * @param [in] u16_Id The Id of block to set "on".
  */
void _set_bitmap(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id)
{
    uint8_t u8_ByteIdx, u8_BitIdx, u8_BitMask;
	u8_ByteIdx = u16_Id/8; // max 8192
	u8_BitIdx = u16_Id %8; // from 0 to 7
	u8_BitMask = 0b10000000 >> u8_BitIdx;
	pCtx->aBitMap[u8_ByteIdx] |= u8_BitMask;
}

/*!
  * @static
  * @brief  Clear the entire bitmap.
  *
  * @param [in] pCtx Pointer on image manager context
  */
void _clr_bitmap(struct img_mgr_ctx_s *pCtx)
{
    uint16_t i;
    for (i=0; i < BITMAP_BLOCK_NB; i++) {
    	pCtx->aBitMap[i] = 0x0;
    }
}

/*!
  * @static
  * @brief  Clear the pending table.
  *
  * @param [in] pCtx Pointer on image manager context
  */
void _clr_pending(struct img_mgr_ctx_s *pCtx)
{
    uint16_t i;
    for (i=0; i < PENDING_LINE_NB; i++)
    {
    	pCtx->sPendCtrl[i].EMPTY = 1;
    	pCtx->sPendCtrl[i].ID = 0;
    	memset(pCtx->sPendBuff[i].aPend,0x00, PENDING_LINE_PEND_BYTES);
    }
}

#ifdef __cplusplus
}
#endif

/*! @} */
