/**
  * @file ext_fw_buffer_template.c
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

#include "ext_fw_buffer_template.h"

#include "parameters_cfg.h"
#include "parameters.h"
#include "app_template.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
//#define HAS_EXTERNAL_FW_UPDATE

/******************************************************************************/
extern admin_ann_fw_info_t sFwAnnInfo;
extern struct update_ctx_s sUpdateCtx;
extern struct adm_config_s sAdmConfig;

#ifdef HAS_EXTERNAL_FW_UPDATE
	static int32_t _fill_adm_ann_received_(buff_io_t *pWorkOut);
	static int32_t _fill_dwn_blk_received_(buff_io_t *pWorkOut);
#endif

static int32_t _fill_inst_complete_(buff_io_t *pWorkOut);
static int32_t _fill_adm_write_complete_(buff_io_t *pWorkOut);

/******************************************************************************/

#ifdef HAS_EXTERNAL_FW_UPDATE
// --- start of #ifdef HAS_EXTERNAL_FW_UPDATE

static ext_fw_buffer_t sExtBuffer;
static uint16_t blk_miss_write_cnt;
static uint8_t blk_available_cnt;

void ExtApi_fw_buffer_init(void)
{
	sExtBuffer.sum_read = 0;
	sExtBuffer.sum_write = 0;
	sExtBuffer.idx_read = 0;
	sExtBuffer.idx_write = 0;

	blk_available_cnt = 0;
	blk_miss_write_cnt = 0;
}

void* ExtApi_fw_buffer_get_rptr(void)
{
	void *ptr = NULL;
	if(sExtBuffer.idx_read != sExtBuffer.idx_write)
	{
		ptr = sExtBuffer.fw_blk[sExtBuffer.idx_read++];
		if(sExtBuffer.idx_read > NB_BLK_STORE_MAX)
		{
			sExtBuffer.idx_read = 0;
		}
		sExtBuffer.sum_read++;
	}
	// else // EMPTY then return NULL;
	return ptr;
}

void* ExtApi_fw_buffer_get_wptr(void)
{
	void *ptr = NULL;
	if(sExtBuffer.idx_write != sExtBuffer.idx_read)
	{
		ptr = sExtBuffer.fw_blk[sExtBuffer.idx_write++];
		if(sExtBuffer.idx_write > NB_BLK_STORE_MAX)
		{
			sExtBuffer.idx_write = 0;
		}
		sExtBuffer.sum_write++;
	}
	// else // FULL then return NULL;
	return ptr;
}

void ExtApi_fw_buffer_write(uint16_t u16Id, const uint8_t *pData)
{
	blk_t *pBlk;
	pBlk = (blk_t *)ExtApi_fw_buffer_get_wptr();
	if (pBlk)
	{
		// Store blk
		memcpy(pBlk->blk, pData, BLK_SZ);
		*((uint16_t*)pBlk->blk_num) = __htons(u16Id);
		blk_available_cnt++;
		/* Notify "external" that a download block is available */
		/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_DWN_BLK_RECV); */
	}
	else
	{
		blk_miss_write_cnt++;
	}
}

// --- end of #ifdef HAS_EXTERNAL_FW_UPDATE
#endif

/******************************************************************************/
#ifdef HAS_EXTERNAL_FW_UPDATE
// --- start of #ifdef HAS_EXTERNAL_FW_UPDATE

static
int32_t _fill_adm_ann_received_(uint8_t *pOut)
{
	uint8_t *p;
	if (pOut)
	{
		if ( sUpdateCtx.ePendUpdate == UPD_PEND_EXTERNAL)
		{
			p = pOut;
			*(uint32_t*)p = __htonl(sFwAnnInfo.u32DwnId);
			((uint32_t*)p)++;
			*(uint16_t*)p = __htonl(sFwAnnInfo.u16SwVerIni);
			((uint16_t*)p)++;
			*(uint16_t*)p = __htonl(sFwAnnInfo.u16SwVerTgt);
			((uint16_t*)p)++;
			*(uint16_t*)p = __htonl(sFwAnnInfo.u16DcHwId);
			((uint16_t*)p)++;
			*(uint16_t*)p = __htonl(sFwAnnInfo.u16BlkCnt);
			((uint16_t*)p)++;
			*p = sFwAnnInfo.u8DayRepeat;
			p++;
			*p = sFwAnnInfo.u8DeltaSec;
			p++;
			*(uint32_t*)p = __htonl(sFwAnnInfo.u32DaysProg);
			((uint32_t*)p)++;
			*(uint32_t*)p = __htonl(sFwAnnInfo.u32HashSW);
			((uint32_t*)p)++;
			return (p - pOut);
		}
	}
	return -1;
}

static
int32_t _fill_dwn_blk_received_(uint8_t *pOut)
{
	uint8_t *p;
	blk_t *pBlk;
	if (pOut)
	{
		if ( sUpdateCtx.ePendUpdate == UPD_PEND_EXTERNAL)
		{
			p = pOut;
			pBlk = (blk_t *)ExtApi_fw_buffer_get_rptr();
			if (pBlk)
			{
				*p = blk_available_cnt;
				blk_available_cnt--;
				p += 1;
				memcpy(p, pBlk->blk_num, 2);
				p += 2;
				memcpy(p, pBlk->blk, BLK_SZ);
				p += BLK_SZ;
				return (p - pOut);
			}
			// Empty
			// else { }
		}
	}
	return -1;
}

// --- end of #ifdef HAS_EXTERNAL_FW_UPDATE
#endif

static
int32_t _fill_inst_complete_(uint8_t *pOut)
{
	uint8_t *p;
	uint8_t i, j, sz;

	if (pOut)
	{
		sz = Param_GetSize(PING_REPLY1);
		p = pOut;
		Param_Access(PING_NBFOUND, p, 0);
		j = 8;
		if (*p < 8) { j = *p; }
		p++;

		for (i = 0; i < j; i++)
		{
			Param_Access(PING_REPLY1 + i, p, 0);
			p += sz;
		}
		return (p - pOut);
	}
	return -1;
}

static
int32_t _fill_adm_write_complete_(uint8_t *pOut)
{
	uint8_t *p;
	uint8_t i;
	if (pOut)
	{
		p = pOut;
		*p = sAdmConfig.u8LastWriteParamNb;
		p++;
		for (i = 0; i < sAdmConfig.u8LastWriteParamNb; i++)
		{
			*p = sAdmConfig.pLastWriteParamIds[i];
			p++;
		}
		return (p - pOut);
	}
	return -1;
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif
