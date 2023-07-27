/**
  * @file proto.c
  * @brief This file implement the Wize Protocol build and extract functionalities
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
  * @par 1.0.0 : 2020/09/07 [GBI]
  * Initial version
  *
  */

/*!
 * @addtogroup wize_proto
 * @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "proto.h"
#include "proto_private.h"
#include "proto_api.h"
#include "crypto.h"
#include "crc_sw.h"
#include "rs.h"
#include <string.h>
#include <time.h>
#include <machine/endian.h>

/******************************************************************************/
//#ifdef WIZE_OPT_USE_CONST_ERR_MSG
/*!
 * @brief This table defines the protocol error string messages
 */
const char * const wize_err_msg[] = {
    [PROTO_SUCCESS]            = "SUCCESS",
    [PROTO_FAILED]             = "FAILED",
    /* Internal Stack related */
    [PROTO_STACK_MISMATCH_ERR] = "Incoherent call/parameters between layers",
    [PROTO_INTERNAL_CRC_ERR]   = "Internal CRC error computation",
    [PROTO_INTERNAL_HASH_ERR]  = "Internal hash error computation",
    [PROTO_INTERNAL_CIPH_ERR]  = "Internal cipher or decipher error computation",
    [PROTO_INTERNAL_NULL_ERR]  = "Error due to a NULL pointer",
    // error_e
    [PROTO_HEAD_END_AUTH_ERR]  = "Head-End authentication failed (Hash Kenc, Klog, Kchg computation result is not valid)",
    [PROTO_GATEWAY_AUTH_ERR]   = "Gateway authentication failed (Hash Kmac  computation result is not valid)",
    [PROTO_PROTO_UNK_ERR]      = "The Protocol is unknown (not Wize, CiField doesn't match).",
    [PROTO_FRAME_UNK_ERR]      = "The Frame is unknown (not Wize, CField doesn't match).",
    [PROTO_FRAME_CRC_ERR]      = "The Frame is corrupted (CRC computation is not valid).",
    [PROTO_FRAME_RS_ERR]       = "The Frame is corrupted (RS could not correct the frame).",
    [PROTO_FRAME_SZ_ERR]       = "The Frame size is too short (< 12) or too long ( > 127, but != 255)",
    [PROTO_APP_MSG_SZ_ERR]     = "The APP message length is too long",
    [PROTO_KEYID_UNK_ERR]      = "The key id is unknown",
    [PROTO_NETWID_UNK_ERR]     = "The Network id is unknown or not match",
    // warning_e
    [PROTO_GATEWAY_AUTH_WRN]   = "Don't know how to authenticate the Gateway (Kmac selection OprID/NetwID)",
    [PROTO_DOWNLOAD_VER_WRN]   = "The Download version is invalid (not register from previous)",
    // info_e
    [PROTO_DW_BLK_PASS_INF]    = "The Download block id is bypassed (already down loaded)",
    [PROTO_FRAME_PASS_INF]     = "The received frame was bypass (we are not the destination device or didn't received an anndownload or not waiting PONG)",
};
//#endif // WIZE_OPT_USE_CONST_ERR_MSG

static uint8_t _encrypt_(uint8_t *p_In, uint8_t u8_Sz,
                         uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);
static uint8_t _decrypt_(uint8_t *p_In, uint8_t u8_Sz,
                         uint8_t p_Ctr[CTR_SIZE], uint8_t u8_KeyId);

static uint8_t _download_extract(struct proto_ctx_s *pCtx, net_msg_t *pNetMsg);
static uint8_t _exchange_extract(struct proto_ctx_s *pCtx, net_msg_t *pNetMsg);
static uint8_t _exchange_build(struct proto_ctx_s *pCtx, net_msg_t *pNetMsg);

/******************************************************************************/

/*!
 * @cond INTERNAL
 * @{
 */

#ifdef WIZE_PROTO_HAS_GETBITMAP
uint8_t DM_GetBitmap(uint16_t u16_Id) __attribute__(( weak ))
{
    return 0;
}
#endif


#define PADDING_SZ ( CTR_SIZE - (L2DWNID_SZ + L6_DWN_VERS_SZ + L6_DWN_B_NUM_SZ) )

/*!
 * @}
 * @endcond
 */

/******************************************************************************/

/*!
  * @brief This function extract the Presentation and Link Layer. The resulting
  * Application Layer is set into the given net_msg_t buffer.
  *
  * @param [in,out] *pCtx Pointer on structure that hold the protocol context.
  * @param [in,out] *pNetMsg Pointer on structure that hold the Application message.
  *
  * @retval PROTO_SUCCESS (see @link ret_code_e::PROTO_SUCCESS @endlink)
  * @retval PROTO_FRAME_RS_ERR (see @link ret_code_e::PROTO_FRAME_RS_ERR @endlink)
  * @retval PROTO_DW_BLK_PASS_INF (see @link ret_code_e::PROTO_DW_BLK_PASS_INF @endlink)
  * @retval PROTO_DOWNLOAD_VER_WRN (see @link ret_code_e::PROTO_DOWNLOAD_VER_WRN @endlink)
  * @retval PROTO_FRAME_CRC_ERR (see @link ret_code_e::PROTO_FRAME_CRC_ERR @endlink)
  * @retval PROTO_HEAD_END_AUTH_ERR (see @link ret_code_e::PROTO_HEAD_END_AUTH_ERR @endlink)
  * @retval PROTO_GATEWAY_AUTH_WRN (see @link ret_code_e::PROTO_GATEWAY_AUTH_WRN @endlink)
  * @retval PROTO_PROTO_UNK_ERR (see @link ret_code_e::PROTO_PROTO_UNK_ERR @endlink)
  * @retval PROTO_KEYID_UNK_ERR (see @link ret_code_e::PROTO_KEYID_UNK_ERR @endlink)
  * @retval PROTO_NETWID_UNK_ERR (see @link ret_code_e::PROTO_NETWID_UNK_ERR @endlink)
  * @retval PROTO_FRAME_PASS_INF (see @link ret_code_e::PROTO_FRAME_PASS_INF @endlink)
  * @retval PROTO_FRAME_UNK_ERR (see @link ret_code_e::PROTO_FRAME_UNK_ERR @endlink)
  * @retval PROTO_FRAME_SZ_ERR (see @link ret_code_e::PROTO_FRAME_SZ_ERR @endlink)
  * @retval PROTO_APP_MSG_SZ_ERR (see @link ret_code_e::PROTO_APP_MSG_SZ_ERR @endlink)
  * @retval PROTO_STACK_MISMATCH_ERR (see @link ret_code_e::PROTO_STACK_MISMATCH_ERR @endlink)
  * @retval PROTO_INTERNAL_CRC_ERR (see @link ret_code_e::PROTO_INTERNAL_CRC_ERR @endlink)
  * @retval PROTO_INTERNAL_HASH_ERR (see @link ret_code_e::PROTO_INTERNAL_HASH_ERR @endlink)
  * @retval PROTO_INTERNAL_CIPH_ERR  (see @link ret_code_e::PROTO_INTERNAL_CIPH_ERR @endlink)
  *
  */
uint8_t Wize_ProtoExtract(
		struct proto_ctx_s *pCtx,
		net_msg_t          *pNetMsg
		)
{
	uint8_t u8Ret = PROTO_INTERNAL_NULL_ERR;
    if (pCtx && pNetMsg && pCtx->pBuffer && pNetMsg->pData)
    {
		pNetMsg->u8Type = APP_UNKNOWN;
		pNetMsg->u8Size = 0;

		pCtx->pBuffer[0] = pCtx->u8Size;
		// Download frame
		if (pCtx->u8Size == 0xFF )
		{
			u8Ret = _download_extract(pCtx, pNetMsg);
		}
		// all other
		else
		{
			// here pCtx->u8Size is the size of L2 frame (i.e. L-Field)
			if (pCtx->u8Size > (L2_EXCH_FIELDS_LEN + L6_EXCH_FIELDS_LEN) )
			{
				// u8RecvLenMax + L6 fields length + L2 fields length (without LField)
				if (pCtx->u8Size <= (pCtx->sProtoConfig.u8RecvLenMax + L2_EXCH_FIELDS_LEN + L6_EXCH_FIELDS_LEN) )
				{
					u8Ret = _exchange_extract(pCtx, pNetMsg);
				}
				else
				{
					u8Ret = PROTO_APP_MSG_SZ_ERR;
				}
			}
			else
			{
				u8Ret = PROTO_FRAME_SZ_ERR;
			}
		}
    }
    return u8Ret;
}

/*!
  * @brief This function build the Presentation and Link Layer. The Application
  * Layer must be into the given net_msg_t buffer
  *
  * @param [in,out] *pCtx Pointer on structure that hold the protocol context.
  * @param [in,out] *pNetMsg Pointer on structure that hold the Application message.
  *
  * @retval PROTO_SUCCESS (see @link ret_code_e::PROTO_SUCCESS @endlink)
  * @retval PROTO_FRAME_SZ_ERR (see @link ret_code_e::PROTO_FRAME_SZ_ERR @endlink)
  * @retval PROTO_APP_MSG_SZ_ERR (see @link ret_code_e::PROTO_APP_MSG_SZ_ERR @endlink)
  * @retval PROTO_STACK_MISMATCH_ERR (see @link ret_code_e::PROTO_STACK_MISMATCH_ERR @endlink)
  * @retval PROTO_INTERNAL_CIPH_ERR (see @link ret_code_e::PROTO_INTERNAL_CIPH_ERR @endlink)
  * @retval PROTO_INTERNAL_HASH_ERR (see @link ret_code_e::PROTO_INTERNAL_HASH_ERR @endlink)
  * @retval PROTO_INTERNAL_CRC_ERR (see @link ret_code_e::PROTO_INTERNAL_CRC_ERR @endlink)
  *
  */
uint8_t Wize_ProtoBuild(
		struct proto_ctx_s *pCtx,
		net_msg_t          *pNetMsg
		)
{
    uint8_t u8Ret = PROTO_INTERNAL_NULL_ERR;

    if (pCtx && pNetMsg && pCtx->pBuffer && pNetMsg->pData)
    {
		pCtx->u8Size = 0;

		// here pNetMsg->u8Size is the size of L7 frame
		if (pNetMsg->u8Size > 0x0)
		{
			if (pNetMsg->u8Size <= pCtx->sProtoConfig.u8TransLenMax)
			{
				u8Ret = _exchange_build(pCtx, pNetMsg);
				if (!u8Ret)
				{
					pCtx->u8Size = pCtx->pBuffer[0];
				}
			}
			else
			{
				u8Ret = PROTO_APP_MSG_SZ_ERR;
			}
		}
		else
		{
			u8Ret = PROTO_APP_MSG_SZ_ERR;
			//u8Ret = PROTO_FRAME_SZ_ERR;
		}
    }
    return u8Ret;
}

/*!
  * @brief This function update the reception statistics.
  *
  * @param [in,out] *pCtx     Pointer on structure that hold the protocol context.
  * @param [in]     u8ErrCode Protocol error code returned from previous Wize_ProtoExtract call.
  * @param [in]     u8Rssi    RSSI from previous reception.
  *
  * @retval None
  *
  */
void Wize_ProtoStats_RxUpdate(
		struct proto_ctx_s *pCtx,
		uint8_t            u8ErrCode,
		uint8_t            u8Rssi
		)
{
	if (pCtx)
	{
		struct proto_stats_s *pStats = &(pCtx->sProtoStats);
		pStats->u32RxNbBytes += pCtx->u8Size;
		// Update Frame stats
		if (u8ErrCode == PROTO_SUCCESS)
		{
			uint32_t u32_temp;
			// Stats on Rssi
			if(u8Rssi > pStats->u8RxRssiMax) {
				pStats->u8RxRssiMax = u8Rssi;
			}
			if(u8Rssi < pStats->u8RxRssiMin) {
				pStats->u8RxRssiMin = u8Rssi;
			}
			u32_temp = (pStats->u8RxRssiAvg*pStats->u32RxNbFrmOK) + u8Rssi;
			pStats->u32RxNbFrmOK++;
			pStats->u8RxRssiAvg = u32_temp/pStats->u32RxNbFrmOK;
		}
		// Update Frame error stats
		else {
			// Increase the number of received erronous frame
			pStats->u32RxNbFrmErr++;
			// set the stats
			switch (u8ErrCode)
			{
				case PROTO_HEAD_END_AUTH_ERR:
					pStats->sFrmErrStats.u32HeadAuthErr++;
					break;
				case PROTO_GATEWAY_AUTH_ERR:
					pStats->sFrmErrStats.u32GatewayAuthErr++;
					break;
				case PROTO_FRAME_UNK_ERR:
					pStats->sFrmErrStats.u32UnkErr++;
					break;
				case PROTO_FRAME_CRC_ERR:
					pStats->sFrmErrStats.u32CrcErr++;
					break;
				case PROTO_FRAME_RS_ERR:
					pStats->sFrmErrStats.u32RsErr++;
					break;
				case PROTO_FRAME_SZ_ERR:
					pStats->sFrmErrStats.u32LenErr++;
					break;
				case PROTO_APP_MSG_SZ_ERR:
					pStats->sFrmErrStats.u32AppSzErr++;
					break;
				default:
					pStats->sFrmErrStats.u32PassedErr++;
					break;
			}
			// Wize stack error, so drop the frame
		}
	}
}

/*!
  * @brief This function update the transmission statistics.
  *
  * @param [in,out] *pCtx     Pointer on structure that hold the protocol context.
  * @param [in]     u8ErrCode Protocol error code returned from previous
  *                           Wize_ProtoBuild call.
  * @param [in]     u8Noise   Noise from previous transmission.
  *
  * @retval None
  *
  */
void Wize_ProtoStats_TxUpdate(
		struct proto_ctx_s *pCtx,
		uint8_t             u8ErrCode,
		uint8_t             u8Noise
		)
{
	if (pCtx)
	{
		struct proto_stats_s *pStats = &(pCtx->sProtoStats);
		pStats->u32TxNbBytes += pCtx->u8Size;
		// Update Frame stats
		if (u8ErrCode == PROTO_SUCCESS)
		{
			uint32_t u32_temp;
			// Stats on Noise
			if(u8Noise > pStats->u8TxNoiseMax) {
				pStats->u8TxNoiseMax = u8Noise;
			}
			if(u8Noise < pStats->u8TxNoiseMin) {
				pStats->u8TxNoiseMin = u8Noise;
			}
			u32_temp = (pStats->u8TxNoiseAvg*pStats->u32TxNbFrames) + u8Noise;
			pStats->u32TxNbFrames++;
			pStats->u8TxNoiseAvg = u32_temp/pStats->u32TxNbFrames;
		}
	}
}

/*!
  * @brief This function clear the reception statistics.
  *
  * @param [in,out] *pCtx     Pointer on structure that hold the protocol context.
  *
  * @retval None
  *
  */
void Wize_ProtoStats_RxClear(
		struct proto_ctx_s *pCtx
		)
{
	if (pCtx)
	{
		struct proto_stats_s *pStats = &(pCtx->sProtoStats);
		pStats->u8RxRssiMax = 0;
		pStats->u8RxRssiMin = 0;
		pStats->u8RxRssiAvg = 0;
		pStats->u8RxReserved = 0;
		pStats->u32RxNbBytes = 0;
		pStats->u32RxNbFrmOK = 0;
		pStats->u32RxNbFrmErr = 0;
		memset(&(pStats->sFrmErrStats), 0, sizeof(frm_err_stats_t));
	}
}

/*!
  * @brief This function clear the transmission statistics.
  *
  * @param [in,out] *pCtx     Pointer on structure that hold the protocol context.
  *
  * @retval None
  *
  */
void Wize_ProtoStats_TxClear(
		struct proto_ctx_s *pCtx
		)
{
	if (pCtx)
	{
		struct proto_stats_s *pStats = &(pCtx->sProtoStats);
		pStats->u8TxNoiseMax = 0;
		pStats->u8TxNoiseAvg = 0;
		pStats->u8TxNoiseMin = 0;
		pStats->u8TxReserved = 0;
		pStats->u32TxNbFrames = 0;
		pStats->u32TxNbBytes = 0;
	}
}
/*!
  * @brief This function return a pointer on string error.
  *
  * @param [in] eErr  The error code number.
  *
  * @retval Pointer on string error
  *
  */
const char * Wize_Proto_GetStrErr(uint8_t eErr)
{
	if (eErr >= PROTO_RET_CODE_NB)
	{
		return wize_err_msg[PROTO_FAILED];
	}
	return wize_err_msg[eErr];
}
/******************************************************************************/

/*!
  * @static
  * @brief This function extract the Download Layer. The resulting
  * Application Layer is set into the given net_msg_t buffer.
  * buffer.
  *
  * @param [in,out] *pCtx Pointer on structure that hold the protocol context.
  * @param [in,out] *pNetMsg Pointer on structure that hold the Application message.
  *
  * @retval PROTO_SUCCESS (see @link ret_code_e::PROTO_SUCCESS @endlink)
  * @retval PROTO_FRAME_RS_ERR (see @link ret_code_e::PROTO_FRAME_RS_ERR @endlink)
  * @retval PROTO_FRAME_CRC_ERR (see @link ret_code_e::PROTO_FRAME_CRC_ERR @endlink)
  * @retval PROTO_HEAD_END_AUTH_ERR (see @link ret_code_e::PROTO_HEAD_END_AUTH_ERR @endlink)
  * @retval PROTO_FRAME_PASS_INF (see @link ret_code_e::PROTO_FRAME_PASS_INF @endlink)
  * @retval PROTO_DW_BLK_PASS_INF (see @link ret_code_e::PROTO_DW_BLK_PASS_INF @endlink)
  * @retval PROTO_DOWNLOAD_VER_WRN (see @link ret_code_e::PROTO_DOWNLOAD_VER_WRN @endlink)
  * @retval PROTO_INTERNAL_CRC_ERR (see @link ret_code_e::PROTO_INTERNAL_CRC_ERR @endlink)
  * @retval PROTO_INTERNAL_HASH_ERR (see @link ret_code_e::PROTO_INTERNAL_HASH_ERR @endlink)
  * @retval PROTO_INTERNAL_CIPH_ERR (see @link ret_code_e::PROTO_INTERNAL_CIPH_ERR @endlink)
  *
  */
static uint8_t _download_extract(
		struct proto_ctx_s *pCtx,
		net_msg_t          *pNetMsg
		)
{
    uint16_t u16_Crc;
    uint8_t l2_end, l6_start, l6_end;
    uint8_t u8Size;
    uint8_t pCtr[CTR_SIZE];
    uint8_t aHash[CTR_SIZE];

    u8Size = pCtx->pBuffer[0];
    // Add the l2 header size
    l6_start = sizeof(l2_down_header_t) +1;
    l2_end = u8Size - sizeof(l2_down_footer_t) +1;
    l6_end = l2_end - sizeof(l6_down_footer_t);
    l2_down_header_t *pL2h = (l2_down_header_t*)(&(pCtx->pBuffer[1]));
    l2_down_footer_t *pL2f = (l2_down_footer_t*)(&(pCtx->pBuffer[l2_end]));
    l6_down_header_t *pL6h = (l6_down_header_t*)(&(pCtx->pBuffer[l6_start]));
    l6_down_footer_t *pL6f = (l6_down_footer_t*)(&(pCtx->pBuffer[l6_end]));

    uint8_t l7_start = l6_start + sizeof(l6_down_header_t);
    uint8_t l_size = l6_end - l7_start;

    // Compute and applied RS

    RS_Init();
    if ( ! RS_Decode( (uint8_t*)pL2h ) )
    {
        // if RS FAILED (too much error), return error RS corrupted
        return PROTO_FRAME_RS_ERR;
    }

    // compute the CRC
    if ( ! CRC_Compute(pCtx->pBuffer, (u8Size +1 - CRC_SZ - RSCODE_SZ ), &u16_Crc) )
    {
        return PROTO_INTERNAL_CRC_ERR;
    }

    // check if CRC match
    if ( ! CRC_Check(__ntohs(*((uint16_t*)(pL2f->Crc))), u16_Crc) )
    {
        return PROTO_FRAME_CRC_ERR;
    }

    // check that current download is match : 0x00 0x00 0x00 : no download pending
    if (pCtx->sProtoConfig.filterDisL2_b.DownId == 0 )
    {
        if ( memcmp(pCtx->sProtoConfig.DwnId, pL2h->L2DownId, L2DWNID_SZ ) )
        {
            // currently no download or dwnid doesn't match, so pass the frame
            return PROTO_FRAME_PASS_INF;
        }
    }

    // check download version
    if ( pL6h->L6DownVer != L6_DOWNLOAD_VER )
    {
        return PROTO_DOWNLOAD_VER_WRN;
    }

    // compute HKlog
    memcpy(pCtr, &(pCtx->pBuffer[1]), CTR_SIZE);
    if ( Crypto_AES128_CMAC( aHash, &(pCtx->pBuffer[l7_start + PADDING_SZ]), l_size - PADDING_SZ, pCtr, KEY_LOG_ID ) != CRYPTO_OK)
    {
        return PROTO_INTERNAL_HASH_ERR;
    }
    // check Hash Klog
    if ( memcmp( pL6f->L6HashLog, aHash, L6_HASH_KLOG_SZ) )
    {
        return PROTO_HEAD_END_AUTH_ERR;
    }

    // uncipher
    memcpy(&(pCtr[0]), pL2h->L2DownId, L2DWNID_SZ);
    memcpy(&(pCtr[L2DWNID_SZ]), pL6h->L6DownBnum, L6_DWN_B_NUM_SZ);
    memset( &(pCtr[L2DWNID_SZ + L6_DWN_B_NUM_SZ]), 0x00, CTR_SIZE - (L2DWNID_SZ + L6_DWN_B_NUM_SZ));

    if (_decrypt_( &(pCtx->pBuffer[l7_start]), l_size, pCtr, KEY_LOG_ID) != CRYPTO_OK)
    {
        return PROTO_INTERNAL_CIPH_ERR;
    }

    // fill net_msg
    pNetMsg->u16Id = __ntohs( *((uint16_t*)(&pL6h->L6DownBnum[1])));
    pNetMsg->u8KeyId = KEY_LOG_ID;
    pNetMsg->u8Size = l_size;
    memcpy(pNetMsg->pData, &(pCtx->pBuffer[l7_start]), l_size);

    pNetMsg->u8Type = APP_DOWNLOAD;
    time_t t;
    time(&t);
    pNetMsg->u32Epoch = t;
    pNetMsg->u16Tstamp = (uint16_t)(pNetMsg->u32Epoch);

    return PROTO_SUCCESS;
}

/*!
  * @static
  * @brief This function extract the Exchange Layer. The resulting
  * Application Layer is set into the given net_msg_t buffer.
  * buffer.
  *
  * @param [in,out] *pCtx Pointer on structure that hold the protocol context.
  * @param [in,out] *pNetMsg Pointer on structure that hold the Application message.
  *
  * @retval PROTO_SUCCESS (see @link ret_code_e::PROTO_SUCCESS @endlink)
  * @retval PROTO_FRAME_CRC_ERR (see @link ret_code_e::PROTO_FRAME_CRC_ERR @endlink)
  * @retval PROTO_HEAD_END_AUTH_ERR (see @link ret_code_e::PROTO_HEAD_END_AUTH_ERR @endlink)
  * @retval PROTO_GATEWAY_AUTH_WRN (see @link ret_code_e::PROTO_GATEWAY_AUTH_WRN @endlink)
  * @retval PROTO_PROTO_UNK_ERR (see @link ret_code_e::PROTO_PROTO_UNK_ERR @endlink)
  * @retval PROTO_KEYID_UNK_ERR (see @link ret_code_e::PROTO_KEYID_UNK_ERR @endlink)
  * @retval PROTO_NETWID_UNK_ERR (see @link ret_code_e::PROTO_NETWID_UNK_ERR @endlink)
  * @retval PROTO_FRAME_PASS_INF (see @link ret_code_e::PROTO_FRAME_PASS_INF @endlink)
  * @retval PROTO_FRAME_UNK_ERR (see @link ret_code_e::PROTO_FRAME_UNK_ERR @endlink)
  * @retval PROTO_APP_MSG_SZ_ERR (see @link ret_code_e::PROTO_APP_MSG_SZ_ERR @endlink)
  * @retval PROTO_STACK_MISMATCH_ERR (see @link ret_code_e::PROTO_STACK_MISMATCH_ERR @endlink)
  * @retval PROTO_INTERNAL_CRC_ERR (see @link ret_code_e::PROTO_INTERNAL_CRC_ERR @endlink)
  * @retval PROTO_INTERNAL_HASH_ERR (see @link ret_code_e::PROTO_INTERNAL_HASH_ERR @endlink)
  * @retval PROTO_INTERNAL_CIPH_ERR (see @link ret_code_e::PROTO_INTERNAL_CIPH_ERR @endlink)
  *
  */
static uint8_t _exchange_extract(
		struct proto_ctx_s *pCtx,
		net_msg_t          *pNetMsg
		)
{
    uint16_t u16_Crc;
    uint8_t l2_end, l6_start, l6_end;
    uint8_t u8Size;
    uint8_t pCtr[CTR_SIZE];
    uint8_t aHash[CTR_SIZE];

    u8Size = pCtx->pBuffer[0];
    // Add the l2 header size
    l6_start = sizeof(l2_exch_header_t) +1;
    l2_end = u8Size - sizeof(l2_exch_footer_t) +1;
    l6_end = l2_end - sizeof(l6_exch_footer_t);
    l2_exch_header_t *pL2h = (l2_exch_header_t*)(&(pCtx->pBuffer[1]));
    l2_exch_footer_t *pL2f = (l2_exch_footer_t*)(&(pCtx->pBuffer[l2_end]));
    l6_exch_header_t *pL6h = (l6_exch_header_t*)(&(pCtx->pBuffer[l6_start]));
    l6_exch_footer_t *pL6f = (l6_exch_footer_t*)(&(pCtx->pBuffer[l6_end]));

    uint8_t l7_start = l6_start + sizeof(l6_exch_header_t);
    uint8_t l_size = l6_end - l7_start;

    // Check that CRC match
    if (pCtx->sProtoConfig.filterDisL2_b.Crc == 0 )
    {
		// compute the CRC
		if ( ! CRC_Compute(pCtx->pBuffer, u8Size +1 - CRC_SZ , &u16_Crc) )
		{
			return PROTO_INTERNAL_CRC_ERR;
		}

		// check if CRC match
		if ( ! CRC_Check(__ntohs(*((uint16_t*)(pL2f->Crc))), u16_Crc) )
		{
			return PROTO_FRAME_CRC_ERR;
		}
    }

    // Check that AField match
    if (pCtx->sProtoConfig.filterDisL2_b.AField == 0 )
    {
        if ( memcmp(pL2h->Afield, pCtx->aDeviceAddr, AFIELD_SZ) )
        {
            return PROTO_FRAME_PASS_INF;
        }
    }
    // Check that MFiled match
    if (pCtx->sProtoConfig.filterDisL2_b.MField == 0 )
    {
        if ( memcmp(pL2h->Mfield, pCtx->aDeviceManufID, MFIELD_SZ) )
        {
            return PROTO_FRAME_PASS_INF;
        }
    }

    // Check that CiField match
    if (pCtx->sProtoConfig.filterDisL2_b.CiField == 0 )
    {
        if (pL2h->Cifield !=  WIZE_PROTO_ID )
        {
            return PROTO_PROTO_UNK_ERR;
        }
    }
    // Check the Wize revision
    if (pL6h->L6Ctrl_b.VERS != L6VERS)
    {
        return PROTO_PROTO_UNK_ERR;
    }

    // Check that Network Id matches
    if (pCtx->sProtoConfig.filterDisL6_b.NetId == 0 )
    {
        if ( pL6h->L6NetwId != pCtx->sProtoConfig.u8NetId)
        {
            return PROTO_NETWID_UNK_ERR;
        }
    }

    // check if L2 layer belong to expected one
    if ( pL2h->Cfield != INSTPONG && pL2h->Cfield != COMMAND)
    {
#ifdef HAS_RECV_DATA_ABILITY
    	if ( pL2h->Cfield != DATA && pL2h->Cfield != DATA_PRIO)
#endif
    	{
    		return PROTO_FRAME_UNK_ERR;
    	}
    }

    // Check if ciphered; if not don't care HKenc
    if (pL6h->L6Ctrl_b.KEYSEL == 0 )
    {
        // No ciphering
        // compute HKmac
        if (pCtx->sProtoConfig.filterDisL6_b.HashKmac == 0 )
        {

#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1 || L6VERS == L6VER_WIZE_REV_1_2
            memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
            memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
            memset(&(pCtr[MFIELD_SZ + AFIELD_SZ]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ));
#else
#error "L6VERS == L6VER_WIZE_REV_0_0"
#endif
            if ( Crypto_AES128_CMAC( aHash, (uint8_t*)pL6h, l2_end - l6_start - L6_HASH_KMAC_SZ, pCtr, (uint8_t)(KEY_MAC_ID) ) != CRYPTO_OK)
            {
                return PROTO_INTERNAL_HASH_ERR;
            }
            // check Hash Kmac
            if ( memcmp( pL6f->L6HKmac, aHash, L6_HASH_KMAC_SZ) )
            {
                return PROTO_GATEWAY_AUTH_ERR;
            }
        }
    }
    else if (pL6h->L6Ctrl_b.KEYSEL <= KEY_CHG_ID)
    {
        // compute HKenc
        if (pCtx->sProtoConfig.filterDisL6_b.HashKenc == 0 )
        {
#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1 || L6VERS == L6VER_WIZE_REV_1_2
            memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
            memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
            memcpy(&(pCtr[MFIELD_SZ + AFIELD_SZ]), pL6h->L6Cpt, L6_CPT_SZ);
            memset(&(pCtr[MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ));
#else
#error "L6VERS == L6VER_WIZE_REV_0_0"
#endif
            if ( Crypto_AES128_CMAC( aHash, &(pCtx->pBuffer[l7_start]), l_size, pCtr, (uint8_t)(pL6h->L6Ctrl_b.KEYSEL) ) != CRYPTO_OK)
            {
                return PROTO_INTERNAL_HASH_ERR;
            }
            // check Hash Kenc
            if ( memcmp( pL6f->L6HashKenc, aHash, L6_HASH_KENC_SZ) )
            {
                return PROTO_HEAD_END_AUTH_ERR;
            }
        }
        // compute HKmac
        if (pCtx->sProtoConfig.filterDisL6_b.HashKmac == 0 )
        {
#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1 || L6VERS == L6VER_WIZE_REV_1_2
            memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
            memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
            memset(&(pCtr[MFIELD_SZ + AFIELD_SZ]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ));
#else
#error "L6VERS == L6VER_WIZE_REV_0_0"
#endif
            if ( Crypto_AES128_CMAC( aHash, (uint8_t*)pL6h, l2_end - l6_start - L6_HASH_KMAC_SZ, pCtr, (uint8_t)(KEY_MAC_ID) ) != CRYPTO_OK)
            {
                return PROTO_INTERNAL_HASH_ERR;
            }
            // check Hash Kmac
            if ( memcmp( pL6f->L6HKmac, aHash, L6_HASH_KMAC_SZ) )
            {
                return PROTO_GATEWAY_AUTH_ERR;
            }
        }

        // uncipher
        //if ( pCtx->sProtoConfig.filterDisL6_b.Ciph == 0 )
        {
			memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
			memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
			memcpy(&(pCtr[MFIELD_SZ + AFIELD_SZ]), pL6h->L6Cpt, L6_CPT_SZ);
			memcpy(&(pCtr[MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ]), &(pL2h->Cfield), 1);
			memset(&(pCtr[MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ + 1]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ + 1));
			if (_decrypt_( &(pCtx->pBuffer[l7_start]), l_size, pCtr, (uint8_t)(pL6h->L6Ctrl_b.KEYSEL)) != CRYPTO_OK)
			{
				return PROTO_INTERNAL_CIPH_ERR;
			}
        }
        // else {} // It is assume that the caller will un-cipher the L7
    }
    else
    {
        return PROTO_KEYID_UNK_ERR;
    }

    if ( pL2h->Cfield == INSTPONG)
    {
        pNetMsg->u8Type = APP_INSTALL;
        pNetMsg->i16TxFreqOffset = __ntohs( *((int16_t*)(pL6f->TxFreqOffset)));
        pNetMsg->u32Epoch = __ntohl( *((uint32_t*)(pL6f->Epoch)));
    }
    else
    {
#ifdef HAS_RECV_DATA_ABILITY
    	if ( pL2h->Cfield == DATA)
    	{
    		pNetMsg->u8Type = APP_DATA;
    		// Add L6App
    		l_size++;
    		l7_start--;
    	}
    	else if ( pL2h->Cfield == DATA_PRIO)
    	{
    		pNetMsg->u8Type = APP_DATA_PRIO;
    		// Add L6App
    		l_size++;
    		l7_start--;
    	}
    	else //if ( pL2h->Cfield == COMMAND)
#endif
        {
    		pNetMsg->u8Type = APP_ADMIN;
        }
        pNetMsg->u16Tstamp = __ntohs( *((uint16_t*)(pL6f->L6TStamp)) );
        time_t t;
        time(&t);
        pNetMsg->u32Epoch = t;
    }

    // fill net_msg
    pNetMsg->u16Id = __ntohs( *((uint16_t*)(pL6h->L6Cpt)));
    pNetMsg->u8KeyId = (uint8_t)pL6h->L6Ctrl_b.KEYSEL;
    pNetMsg->u8Size = l_size;
    memcpy(pNetMsg->pData, &(pCtx->pBuffer[l7_start]), l_size);

    return PROTO_SUCCESS;
}

/*!
  * @static
  * @brief This function build the Exchange Layer. The Application
  * Layer must be into the given net_msg_t buffer
  * buffer.
  *
  * @param [in,out] *pCtx Pointer on structure that hold the protocol context.
  * @param [in,out] *pNetMsg Pointer on structure that hold the Application message.
  *
  * @retval PROTO_SUCCESS (see @link ret_code_e::PROTO_SUCCESS @endlink)
  * @retval PROTO_STACK_MISMATCH_ERR (see @link ret_code_e::PROTO_STACK_MISMATCH_ERR @endlink)
  * @retval PROTO_INTERNAL_CIPH_ERR (see @link ret_code_e::PROTO_INTERNAL_CIPH_ERR @endlink)
  * @retval PROTO_INTERNAL_HASH_ERR (see @link ret_code_e::PROTO_INTERNAL_HASH_ERR @endlink)
  * @retval PROTO_INTERNAL_CRC_ERR (see @link ret_code_e::PROTO_INTERNAL_CRC_ERR @endlink)
  */
static uint8_t _exchange_build(
		struct proto_ctx_s *pCtx,
		net_msg_t          *pNetMsg
		)
{
    l2_exch_header_t *pL2h;
    l6_exch_header_t *pL6h;
    l6_exch_footer_t *pL6f;
    l2_exch_footer_t *pL2f;
    uint8_t *pData;
    uint8_t pCtr[CTR_SIZE];
    uint8_t aHash[CTR_SIZE]; // required size due to tinyCrypt hash output is on TC_AES_BLOCK_SIZE which is 4x4
    uint8_t l6_start, l7_start, l6_end, l2_end, l_size, u8Size;
    uint16_t u16Crc;

    //
    u8Size = pNetMsg->u8Size;
    pData = pNetMsg->pData;

    // Check if application payload has L6App
    if (pNetMsg->Option_b.App)
    {
    	u8Size--;
    	pData++;
    }

    l6_start = sizeof(l2_exch_header_t) + 1;
    l7_start = l6_start + sizeof(l6_exch_header_t);
    l6_end = l7_start + u8Size;
    l2_end = l6_end + sizeof(l6_exch_footer_t);
    l_size = l6_end - 1;

    pL2h = (l2_exch_header_t*)(&(pCtx->pBuffer[1]));
    pL6h = (l6_exch_header_t*)(&(pCtx->pBuffer[l6_start]));
    pL6f = (l6_exch_footer_t*)(&(pCtx->pBuffer[l6_end]));
    pL2f = (l2_exch_footer_t*)(&(pCtx->pBuffer[l2_end]));

    pL2h->Cfield = 0;
    switch (pNetMsg->u8Type)
    {
		case APP_INSTALL: // PING only
	        // set the L6App field
	        pL6h->L6App = pCtx->sProtoConfig.AppInst;
	        // Set the CField
	        pL2h->Cfield = INSTPING;
			break;
		case APP_ADMIN: // RESPONSE only
	        // set the L6App field
	        pL6h->L6App = pCtx->sProtoConfig.AppAdm;
	        // Set the CField
	        pL2h->Cfield = RESPONSE;
			break;
		case APP_DATA_PRIO:
	        // Set the CField
	        pL2h->Cfield = DATA_PRIO;
		case APP_DATA:
	        // Set the CField
	        pL2h->Cfield |= DATA;
	        // Check if application payload has L6App
	        if (pNetMsg->Option_b.App)
	        {
	            // set the L6App field
	            pL6h->L6App = *( pData -1);
	        }
	        else
	        {
	            // set the L6App field
	            pL6h->L6App = pCtx->sProtoConfig.AppData;
	        }
			break;
    	default:
    		// APP_DOWNLOAD or APP_UNKNOWN
    		return PROTO_STACK_MISMATCH_ERR;
    		break;
    }


   	*(uint16_t*)(pL6h->L6Cpt) = __htons( pNetMsg->u16Id );
    pL6h->L6NetwId = pCtx->sProtoConfig.u8NetId;

    // Set the L6 fields
    pL6h->L6Ctrl_b.VERS = L6VERS;
    pL6h->L6Ctrl_b.KEYSEL = pNetMsg->u8KeyId;
    pL6h->L6Ctrl_b.WTS = 1;

    // Set the L2 fields
    memcpy( pL2h->Afield, pCtx->aDeviceAddr, AFIELD_SZ );
    memcpy( pL2h->Mfield, pCtx->aDeviceManufID, MFIELD_SZ);
    pL2h->Cifield = WIZE_PROTO_ID;

    memcpy(&(pCtx->pBuffer[l7_start]), pData, u8Size);
    if (pNetMsg->u8KeyId)
    {
    	// Check if application payload is already ciphered
    	if (
    		( pNetMsg->Option_b.Ciph == 0 ) ||
    		( (pNetMsg->u8Type != APP_DATA) && (pNetMsg->u8Type != APP_DATA_PRIO ) )
			)
    	{
			// cipher
			memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
			memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
			memcpy(&(pCtr[MFIELD_SZ + AFIELD_SZ]), pL6h->L6Cpt, L6_CPT_SZ);
			memcpy(&(pCtr[MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ]), &(pL2h->Cfield), 1);
			memset(&(pCtr[MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ + 1]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ + 1));

			if (_encrypt_( &(pCtx->pBuffer[l7_start]), u8Size, pCtr, pNetMsg->u8KeyId) != CRYPTO_OK)
			{
				return PROTO_INTERNAL_CIPH_ERR;
			}
    	}
    	//else {} // It is assume that the caller has already ciphered the L7.
    }

    // compute HKenc
#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1 || L6VERS == L6VER_WIZE_REV_1_2
    memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
    memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
    memcpy(&(pCtr[MFIELD_SZ + AFIELD_SZ]), pL6h->L6Cpt, L6_CPT_SZ);
    memset(&(pCtr[MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ + L6_CPT_SZ));
#else
#error "L6VERS == L6VER_WIZE_REV_0_0"
#endif
    if (pNetMsg->u8KeyId)
    {
        if ( Crypto_AES128_CMAC( aHash, &(pCtx->pBuffer[l7_start]), u8Size, pCtr, (uint8_t)(pL6h->L6Ctrl_b.KEYSEL) ) != CRYPTO_OK)
        {
            return PROTO_INTERNAL_HASH_ERR;
        }
    }
    else
    {	// TODO : Crypto_AES128_CMAC takes stack without release it
        if ( Crypto_AES128_CMAC( aHash, &(pCtx->pBuffer[l7_start]), u8Size, pCtr, (uint8_t)(KEY_MAC_ID) ) != CRYPTO_OK)
        {
            return PROTO_INTERNAL_HASH_ERR;
        }
    }
    // set HKenc
    memcpy( pL6f->L6HashKenc, aHash, L6_HASH_KENC_SZ );

    // Set L6TStamp
    time_t t;
    time(&t);
    *(uint16_t*)(pL6f->L6TStamp) = __htons((uint16_t)t);
    pNetMsg->u16Tstamp = (uint16_t)t;
    pNetMsg->u32Epoch = (uint32_t)t;

    l_size += L6_HASH_KENC_SZ + L6_TSTAMP_SZ;

    // compute HKmac
#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1 || L6VERS == L6VER_WIZE_REV_1_2
    memcpy(&(pCtr[0]), pL2h->Mfield, MFIELD_SZ);
    memcpy(&(pCtr[MFIELD_SZ]), pL2h->Afield, AFIELD_SZ);
    memset(&(pCtr[MFIELD_SZ + AFIELD_SZ]), 0x00, CTR_SIZE - (MFIELD_SZ + AFIELD_SZ));
#else
#error "L6VERS == L6VER_WIZE_REV_0_0"
#endif

    if ( Crypto_AES128_CMAC( aHash, (uint8_t*)pL6h, l_size - sizeof(l2_exch_header_t), pCtr, (uint8_t)(KEY_MAC_ID) ) != CRYPTO_OK)
    //if ( Crypto_AES128_CMAC( aHash, (uint8_t*)pL6h, l2_end - L6_HASH_KMAC_SZ, pCtr, (uint8_t)(KEY_MAC_ID) ) != CRYPTO_OK)
    {
        return PROTO_INTERNAL_HASH_ERR;
    }
    // set Kmac
    memcpy( pL6f->L6HKmac, aHash, L6_HASH_KMAC_SZ );
    l_size += L6_HASH_KMAC_SZ;

    pCtx->pBuffer[0] = l_size + CRC_SZ;

    // compute and set the CRC
    if ( ! CRC_Compute(pCtx->pBuffer, l_size + 1, &u16Crc) )
    {
        return PROTO_INTERNAL_CRC_ERR;
    }
    *(uint16_t*)(pL2f->Crc) = __htons(u16Crc);
    //pCtx->pBuffer[l_size + CRC_SZ +1] = '\0';

    return PROTO_SUCCESS;
}

/*!
  * @static
  * @brief Wrapper to Crypto_Encrypt(p_Out, p_In, u8_Sz, p_Ctr, u8_KeyId)
  * function.
  *
  * @param [in,out] *p_In point on data to encrypt (plaintext). This will be
  * replaced by the encrypted one.
  * @param [in] u8_Sz     length, in byte, of the data stream to encrypt.
  * @param [in] p_Ctr     initial value for the AES counter.
  * @param [in] u8_KeyId  the key id to be used for encryption.
  * @return See the return codes from @link Crypto_Encrypt @endlink function.
  */
static uint8_t _encrypt_(
		uint8_t *p_In,
		uint8_t u8_Sz,
        uint8_t p_Ctr[CTR_SIZE],
		uint8_t u8_KeyId
		)
{
    uint8_t ret;
    uint8_t p_Out[u8_Sz];
    ret = Crypto_Encrypt(p_Out, p_In, u8_Sz, p_Ctr, u8_KeyId);
    if (ret == CRYPTO_OK) {
        memcpy(p_In, p_Out, u8_Sz);
    }
    return ret;
}

/*!
  * @static
  * @brief Wrapper to Crypto_Decrypt(p_Out, p_In, u8_Sz, p_Ctr, u8_KeyId)
  * function.
  *
  * @param [in,out] *p_In point on data to decrypt (plaintext). This will be
  * replaced by the decrypted one.
  * @param [in] u8_Sz     length, in byte, of the data stream to decrypt.
  * @param [in] p_Ctr     initial value for the AES counter.
  * @param [in] u8_KeyId  the key id to be used for decryption.
  * @return See the return codes from @link Crypto_Decrypt @endlink function.
  */
static uint8_t _decrypt_(
		uint8_t *p_In,
		uint8_t u8_Sz,
        uint8_t p_Ctr[CTR_SIZE],
		uint8_t u8_KeyId
		)
{
    uint8_t ret;
    uint8_t p_Out[u8_Sz];
    ret = Crypto_Decrypt(p_Out, p_In, u8_Sz, p_Ctr, u8_KeyId);
    if (ret == CRYPTO_OK) {
        memcpy(p_In, p_Out, u8_Sz);
    }
    return ret;
}

#ifdef __cplusplus
}
#endif

/*! @} */
