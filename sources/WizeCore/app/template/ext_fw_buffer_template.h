/**
  * @file ext_fw_buffer_template.h
  * @brief // TODO This file ...
  * 
  * @details
  *
  * @copyright 2022, GRDF, Inc.  All rights reserved.
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
  * @par 1.0.0 : 2023/01/17 [GBI]
  * Initial version
  *
  */
#ifndef _EXT_FW_BUFFER_TEMPLATE_H_
#define _EXT_FW_BUFFER_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/******************************************************************************/

/******************************************************************************/
#ifdef HAS_EXTERNAL_FW_UPDATE
// --- start of #ifdef HAS_EXTERNAL_FW_UPDATE

#define NB_BLK_STORE_MAX 64
#define BLK_SZ 210

__attribute__((packed))
typedef struct  blk_s
{ //
	uint8_t blk_num[2];
	uint8_t blk[BLK_SZ];
} blk_t;

typedef struct
{
	uint8_t idx_write;
	uint8_t idx_read;

	uint16_t sum_write;
	uint16_t sum_read;
	uint8_t fw_blk[NB_BLK_STORE_MAX][sizeof(blk_t)];
} ext_fw_buffer_t;

void ExtApi_fw_buffer_init(void);
void* ExtApi_fw_buffer_get_rptr(void);
void* ExtApi_fw_buffer_get_wptr(void);
void ExtApi_fw_buffer_write(uint16_t u16Id, const uint8_t *pData);

// --- end of #ifdef HAS_EXTERNAL_FW_UPDATE
#endif
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _EXT_FW_BUFFER_TEMPLATE_H_ */
