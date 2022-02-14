/**
  * @file: img_storage_private.h
  * @brief: This file define data structure of the image storage module.
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
  * 1.0.0 : 2021/08/25[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Samples
 * @{
 *
 */
#ifndef _IMG_STORAGE_PRIVATE_H_
#define _IMG_STORAGE_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "img_storage.h"
#include <stdint.h>
#include <stddef.h>

#ifndef FLASH_PAGE_SIZE
/*!
 * @def FLASH_PAGE
 * @brief Define the size of one page of FLASH memory
 */
    #define FLASH_PAGE_SIZE 2048
#endif

#ifndef FLASH_START_ADDRESS
/*!
 * @def FLASH_START_ADDRESS
 * @brief Define the beginning of the FLASH memory
 */
    #define FLASH_START_ADDRESS 0x08000000
#endif

#ifndef IMG_ADRR
/*!
 * @def IMG_ADRR
 * @brief Define the beginning of the software binary image (FLASH_PAGE aligned).
 */
	#define IMG_ADRR 0x0802C000
#endif

#ifndef IMG_MAX_SZ
/*!
 * @def IMG_MAX_SZ
 * @brief Maximum size allowed to a software binary image (FLASH_PAGE aligned).
 */
	//#define IMG_MAX_SZ 262144 //!< This is arbitrarily fixed 256ko (half of hyp.512ko flash memory)

	#define IMG_MAX_SZ 169984 //!< This is fixed to 83 pages (1/3 of hyp.512ko flash memory), 14336 bytes remains,
#endif


#define IMG_NB_PAGES IMG_MAX_SZ/FLASH_PAGE_SIZE // 128

/*!
 * @brief Convenient macro to get the first address of a flash page
 */
#define GET_START_PAGE_ADDRESS(page) (FLASH_START_ADDRESS + (0x800 *page) )

/*!
 * @brief Convenient macro to get the last address of a flash page
 */
#define GET_END_PAGE_ADDRESS(page) (FLASH_START_ADDRESS + (0x7FF * (page+1)) )

/*
 * Note :
 * - STM32 Flash memory must be programmed with 64 bit word aligned on double word
 *
 * - Software block (download) are fix size of 210 bytes
 *   - 4 block of 210 bytes are required to fully write 8 bytes aligned into the flash memory
 *
 * - Let call "line" a 4 blocks, with a block 0, 1, 2 and 3 relative to this line
 *   - In this "line" it exist 3 parts of unaligned to double-word
 *     - Between the end of block[0] (2 bytes) and the beginning of block[1] (6 bytes)
 *     - Between the end of block[1] (4 bytes) and the beginning of block[2] (4 bytes)
 *     - Between the end of block[2] (6 bytes) and the beginning of block[3] (2 bytes)
 *
 * - The maximum block number is 800, that is 200 lines (200 * 3 * 8 bytes = 4800 bytes,
 *  which is relatively too large).
 *   - The number of line is reduced to 200/8 = 25 (25 * 3 * 8 = 600 bytes)
 *   - So, in the pending table 25 entries of 8 bytes
 *   - Each entry is allocated to 8 different line
 * - Expecting that missed block are uncommon
 * - Expecting that missed block will be re-emitted
 *
 * - Entries in the pending table are as the following :
 *   - int16_t     i16Id : id of the first pending block
 *   - uint8_t aPend[24] : 3 x pending double-word
 *
 * - If an entry is already takes by an other block id, then the block is dropped
 *
 */

/*!
 * @def BANK_SZ
 * @brief Fix size allowed to a software bank.
 */
#define BANK_SZ IMG_MAX_SZ

/*!
 * @def BLOCK_SZ
 * @brief Fix size of download block.
 */
#define BLOCK_SZ 210

/*!
 * @def BLOCK_NB
 * @brief Define the maximum number of block of the image sw.
 */
#define BLOCK_NB ( BANK_SZ/BLOCK_SZ ) // 800 block

/*!
 * @def BITMAP_BLOCK_NB
 * @brief Define the maximum number of block of bitmap allowed.
 */
#define BITMAP_BLOCK_NB ( BLOCK_NB/8 ) // 100 = 800 block / 8 bits

#define PENDING_LINE_NB 25 //(BANK_SZ/BLOCK_SZ)/4 /8
#define PENDING_LINE_PEND_BYTES 24

/*!
 * @brief This structure hold a pending entry
 */
struct sw_pend_buff_s {
	uint8_t aPend[PENDING_LINE_PEND_BYTES]; /*!< Pending bytes  */
} __attribute__ ((aligned (8)));

/*!
 * @brief This structure hold a pending entry control
 */
struct sw_pend_ctrl_s {
	union {
		uint16_t u16Id;
		struct
		{
			uint16_t ID:15;                    /*!< Id of the first pending block */
			uint16_t EMPTY:1;                  /*!< Line is empty  */
		};
	};
};

/*!
 * @brief This structure hold a image manager context
 */
struct img_mgr_ctx_s {
	struct sw_pend_buff_s sPendBuff[PENDING_LINE_NB]; /*!< Pending table buffer  */
	struct sw_pend_ctrl_s sPendCtrl[PENDING_LINE_NB]; /*!< Pending table control */

	uint8_t aBitMap[BITMAP_BLOCK_NB];    /*!< Bitmap of already get block
	 	 	 	 	 	 	 	 	 	      Each bit representing a block.
	 	 	 	 	 	 	 	 	 	      at index 0, MSB is block 0 ad LSB block 7
	 	 	 	 	 	 	 	 	 	      at index 1, MSB is block 8 ad LSB block 15
	 	 	 	 	 	 	 	 	 	      and so on.
	 	 	 	 	 	 	 	 	 	  */
	uint32_t u32Addr;                     /*!< Start address of the image SW area */
	uint32_t u32Size;                     /*!< Size of the image SW area */

	uint32_t u32NbExpectedBlk;            /*!< Number of expected block */
	uint32_t u32NbBlk;                    /*!< Current total number of block */

	img_pend_e eImgPend;                  /*!< Indicate if img is already pending*/
	uint8_t (*write)(uint32_t u32Addr, uint64_t *pData, uint32_t u32NbDoubleWord);
	uint8_t (*erase)(uint32_t u32Addr, uint32_t u32Size);
};

/*!
 * @brief This struct defines the image context type.
 */
typedef struct img_mgr_ctx_s img_mgr_ctx_t;

#ifdef __cplusplus
}
#endif
#endif /* _IMG_STORAGE_PRIVATE_H_ */

/*! @} */
