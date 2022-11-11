/**
  * @file wize_app.c
  * @brief // TODO This file ...
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
  * @par 1.0.0 : 2022/11/05 [GBI]
  * Initial version
  *
  */

#ifdef __cplusplus
extern "C" {
#endif

#include "wize_app.h"

#include "wize_api.h"

#include "inst_internal.h"
#include "adm_internal.h"

#include "parameters.h"
#include "parameters_lan_ids.h"

#include "ses_dispatcher.h"

/*!
 * @addtogroup wize_app
 * @{
 *
 */

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

// Must be saved
static uint16_t _u16PeriodInst_; // in days (from param EXECPING_PERIOD convert from month to days)
// Must be saved
static uint16_t _u16FullPower_; // in days

static uint32_t _u32InitMask_;
static uint8_t  _bPendAction_;
static struct ping_reply_ctx_s _sPingReply_;

static net_msg_t *_pCmdMsg_;
static net_msg_t *_pRspMsg_;
static net_msg_t *_pPongMsg_;
static net_msg_t *_pBlkMsg_;

/******************************************************************************/

static inline uint8_t _is_periodic_inst_req_(void);
static inline uint8_t _is_full_power_req_(void);

typedef enum
{
	WIZEAPP_ADM_CMD_PEND  = 0x01,
	WIZEAPP_ADM_RSP_PEND  = 0x02,
} wizeapp_adm_pending_e;

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/*!
 * @brief This function
 *
 * @return
 *
 */
void WizeApp_Init(net_msg_t *pCmdMsg, net_msg_t *pRspMsg, net_msg_t *pPongMsg, net_msg_t *pBlkMsg)
{
	if(pCmdMsg && pRspMsg && pPongMsg && pBlkMsg)
	{
		_pCmdMsg_  = pCmdMsg;
		_pRspMsg_  = pRspMsg;
		_pPongMsg_ = pPongMsg;
		_pBlkMsg_  = pBlkMsg;
		_u16PeriodInst_ = 0;
		_u16FullPower_ = 0;
		_bPendAction_ = 0;
		_u32InitMask_ = 0xFFFFFFFF;
	}
	else
	{
		_u32InitMask_ = 0;
	}
}

/******************************************************************************/

/*!
 * @brief This function
 *
 * @return
 *
 */
__attribute__((weak))
uint8_t WizeApp_OnDwnBlkRecv(uint16_t u16Id, const uint8_t *pData)
{
	(void)u16Id;
	(void)pData;
	return 0;
}

/*!
 * @brief This function
 *
 * @return
 *
 */
__attribute__((weak))
uint8_t WizeApp_OnDwnComplete(void)
{
	return 0;
}

/*!
 * @brief This function
 *
 * @return
 *
 */
__attribute__((weak))
uint8_t WizeApp_AnnCheckFwInfo(admin_cmd_anndownload_t *pAnn)
{
	(void)pAnn;
	/* TODO :
	 * ANNDOWNLOAD for an external device
	 *
	 * In this case the response is marked as "not yet available". The maximum
	 * delay allowed is less than ExchRespDelay (between 1 and 255 second)
	 *
	 * (If an external MCU exist, then)
	 * - The external MCU should  :
	 *   - Check Blocks Count             : L7BlocksCount
	 *   - Check HW version               : L7DcHwId
	 *   - Check Initial SW version       : L7SwVersionIni
	 *   - Check Target SW version        : L7SwVersionTarget
	 *   - Send back its answer (validate the ANN or not, plus the error code
	 *     as defined by Wize spec.)
	 *
	 * Internal MCU :
	 * - Check that external MCU exist
	 * - Build a kind of external ANN_DOWNLOAD request
	 * - Send this request to the external MCU
	 * - Get the answer from external MCU (if any)
	 * - Merge the External and Internal response
	 * - Notify ADM that response is ready
	 *
	 */
	return 0;
}

/*!
 * @brief This function
 *
 * @return
 *
 */
void WizeApp_AnnReady(uint8_t eErrCode, uint8_t u8ErrorParam)
{
	/*
	 * eErrCode should be in :
	 * - ANN_INCORRECT_HW_VER:
	 *   -> u8ErrorParam = LSB of current HW version.
	 *
	 * - ANN_INCORRECT_INI_SW_VER:
	 *   ANN_TGT_SW_VER:
	 *   ANN_TGT_VER_DWL:
	 *   -> u8ErrorParam = LSB of current SW version.
	 *
	 * - ANN_ILLEGAL_VALUE:
	 *   -> u8ErrorParam = one of admin_ann_param_id_e
	 *
	 * - ANN_UPD_IN_PROGRESS:
	 *   -> u8ErrorParam = ??? (Local vs internal vs external)
	 *
	*/
	admin_rsp_err_t *pRsp;

	if ( _bPendAction_ & WIZEAPP_ADM_RSP_PEND)
	{
		do {
			pRsp = (admin_rsp_err_t *)(_pRspMsg_->pData);
			if ( ( pRsp->L7ResponseId != ADM_ANNDOWNLOAD ) ||
				 ( pRsp->L7ResponseId != _pCmdMsg_->pData[0] ) )
			{
				break;
			}
			if (eErrCode)
			{
				pRsp->L7ErrorCode = eErrCode;
				pRsp->L7ErrorParam = u8ErrorParam;
			}
			// signal that RSP is ready
			WizeApi_Notify(SES_EVT_ADM_READY);
			_bPendAction_ &= (~WIZEAPP_ADM_RSP_PEND);
		} while(0);
	}
}

//void onKeyA() __attribute__((weak, alias("onKeyBlank")));

void WizeApp_Install(void)
{
	// Start Install session
	inst_ping_t sPing = InstInt_Init(&_sPingReply_);
	WizeApi_ExecPing((uint8_t*)&sPing, sizeof(inst_ping_t));
}

/*
void WizeApp_Send(void)
{
	// Start ADM Data session
	WizeApi_Send(pData, u8Size, APP_DATA);
}

void WizeApp_Alarm(void)
{
	// Start ADM Data Prio session
	WizeApi_Send(pData, u8Size, APP_DATA_PRIO);
}

*/

/*!
 * @brief This function
 *
 * @return
 *
 */
uint32_t WizeApp_Common(uint32_t ulEvent)
{
	uint32_t ret = 0;

	ulEvent &= _u32InitMask_;

	// lock
	/**************************************************************************/
	// ADM back flag
	if (ulEvent & SES_FLG_CMD_RECV)
	{
		// treat CMD and prepare RSP
		uint8_t rsp_available = AdmInt_PreCmd( _pCmdMsg_, _pRspMsg_ );

		// response not yet available (EXECPING or ANNDOWN external FW case)
		if ( rsp_available == 0)
		{
			_bPendAction_ |= WIZEAPP_ADM_RSP_PEND;
			switch(_pCmdMsg_->pData[0])
			{
				case ADM_EXECINSTPING:
					// Start Install session
					WizeApp_Install();
					break;
				case ADM_ANNDOWNLOAD:
					break;
					WizeApp_AnnCheckFwInfo( (admin_cmd_anndownload_t*)(_pCmdMsg_->pData));
				default:
					break;
			}
		}
		// response is available or already treated
		else
		{
			if ( rsp_available == 1) // response is available
			{
				_bPendAction_ |= WIZEAPP_ADM_CMD_PEND;
			}
			else if ( rsp_available == 2) // action already done
			{
				_bPendAction_ &= ~(WIZEAPP_ADM_CMD_PEND | WIZEAPP_ADM_RSP_PEND);
			}
			// else { }

			// signal that RSP is ready
			WizeApi_Notify(SES_EVT_ADM_READY);
		}
		// ADM CMD is received, so clear the "full power" counter
		_u16FullPower_ = 0;
		// return last CMD : cmd
		// only if "action" it is not already done
		// ...mark that RSP is pending, so EXECPING or ANN_ext
	}

	if (ulEvent & SES_FLG_RSP_SENT)
	{
		// action must be executed
		if (_bPendAction_ & WIZEAPP_ADM_CMD_PEND)
		{
			// do action and return the last CMD
			ret |= AdmInt_PostCmd( _pCmdMsg_, _pRspMsg_ );
			_bPendAction_ &= (~WIZEAPP_ADM_CMD_PEND);
		}
	}

	if (ulEvent & SES_FLG_ADM_MSK)
	{
		// SES_FLG_ADM_COMPLETE :
		// - each time the ADM session go back in IDLE state

		// SES_FLG_ADM_OUT_DATE
		// - response was not ready, so not send
		// ADM session go back in IDLE state

		// SES_FLG_ADM_TIMEOUT
		// - listening window is closed
		// - failed to send DATA or RSP
		// ADM session go back in IDLE state

		// SES_FLG_ADM_ERROR :
		// - NetMgr Send or Listen call failed
		// - TimeEvt Start call failed
		// - failed to send DATA or RSP
		// ADM session go back in IDLE state
	}

	/**************************************************************************/
	// INST back flag
	if (ulEvent & SES_FLG_PONG_RECV)
	{
		inst_pong_t *pPong = (inst_pong_t *)(_pPongMsg_->pData);
		LOG_DBG(
			"\t<- K: %02x %02x %02x %02x %02x %02x; MLAN: %02x; RSSI up: %02x; dwn: %02x\n"
			, pPong->L7ConcentId[0]
			, pPong->L7ConcentId[1]
			, pPong->L7ConcentId[2]
			, pPong->L7ConcentId[3]
			, pPong->L7ConcentId[4]
			, pPong->L7ConcentId[5]
			, pPong->L7ModemId
			, pPong->L7RssiUpstream
			, _pPongMsg_->u8Rssi
			);
		// treat the received message that should be store in sRspMsg
		InstInt_Add(&_sPingReply_, _pPongMsg_ );
		// signal that PONG msg has been treated, buffer is ready
		WizeApi_Notify(SES_EVT_INST_READY);
	}

	if (ulEvent & SES_FLG_INST_COMPLETE)
	{
		if ( !(ulEvent & SES_FLG_INST_ERROR) )
		{
			InstInt_End( &_sPingReply_ );
			if ( _bPendAction_ & WIZEAPP_ADM_RSP_PEND)
			{
				if (_pCmdMsg_->pData[0] == ADM_EXECINSTPING)
				{
					_bPendAction_ &= (~WIZEAPP_ADM_RSP_PEND);
					if ( AdmInt_PreCmd( _pCmdMsg_, _pRspMsg_ ) == 2)
					{
						// signal that RSP is ready
						WizeApi_Notify(SES_EVT_ADM_READY);
					}
				}
			}
			// Install session is complete without error, so clear the "periodic install" counter
			_u16PeriodInst_ = 0;
		}
	}

	if (ulEvent & SES_FLG_INST_MSK)
	{
		// SES_FLG_INST_COMPLETE :
		// - each time the INST session go back in IDLE state

		// SES_FLG_INST_OUT_DATE
		// - receiving buffer was not ready, so the frame is discarded

		// SES_FLG_INST_TIMEOUT
		// - listening window is closed
		// - failed to send PING
		// INST session go back in IDLE state

		// SES_FLG_INST_ERROR :
		// - NetMgr Send, Listen or ListenReady call failed
		// - TimeEvt Start call failed
		// - failed to send PING
		// INST session go back in IDLE state
	}

	/**************************************************************************/
	// DWN back flag

	if (ulEvent & SES_FLG_BLK_RECV)
	{
		WizeApp_OnDwnBlkRecv( _pBlkMsg_->u16Id, _pBlkMsg_->pData );
		WizeApi_Notify(SES_EVT_DWN_READY);
	}

	if (ulEvent & SES_FLG_DWN_COMPLETE )
	{
		if ( ! (ulEvent & SES_FLG_DWN_ERROR) )
		{
			WizeApp_OnDwnComplete();
		}
	}

	if (ulEvent & SES_FLG_DWN_MSK)
	{
		// SES_FLG_DWN_COMPLETE :
		// - each time the DWN session go back in IDLE state

		// SES_FLG_DWN_OUT_DATE
		// - receiving buffer was not ready, so the frame is discarded

		// SES_FLG_DWN_TIMEOUT
		// - listening window is closed
		// DWN session go in WAITING_RX_DELAY state

		// SES_FLG_DWN_ERROR :
		// - NetMgr Listen or SetDwlink call failed
		// DWN session go in LISTENING state
		// - TimeEvt Init or Start call failed
		// DWN session go back in IDLE state
	}

	/**************************************************************************/
	// unlock
	return ret;
}

/*!
 * @brief This function
 *
 * @return
 *
 */
uint32_t WizeApp_Time(uint32_t ulEvent)
{
	uint32_t ret = 0;

	ulEvent &= _u32InitMask_;

	/**************************************************************************/
	// Time back flag
	if (ulEvent & SES_EVT_EXT_MSK)
	{
		if (ulEvent & SES_EVT_DAY_PASSED)
		{

			if ( _is_full_power_req_() )
			{
				// Full power has been set
				// Just info
				ret |= WIZEAPP_INFO_FULL_POWER;
			}
			if ( _is_periodic_inst_req_() )
			{
				// Periodic install is required
				// TODO :
				//if (WizeApp_Install() == WIZE_API_SUCCESS)
				//{
				//	ret |= WIZEAPP_INFO_PERIO_INST;
				//}
				WizeApp_Install();
			}
		}
		ret |= WIZEAPP_INFO_DAY_PASSED;
	}
	/**************************************************************************/
	return ret;
}

/******************************************************************************/

/*!
 * @static
 * @brief This function update "Periodic Install" counter
 *
 * @return 1 if periodic install is required, 0 otherwise
 *
 */
static inline uint8_t _is_periodic_inst_req_(void)
{
	uint8_t temp;
	uint16_t v;

	Param_Access(EXECPING_PERIODE, &temp, 0);
	v = temp*30;
	if (_u16PeriodInst_ >= v)
	{
		// request for periodic install
		return 1;
	}
	else
	{
		_u16PeriodInst_++;
		return 0;
	}
}

/*!
 * @static
 * @brief This function update "Full Power" counter
 *
 * @return 1 if periodic install is required, 0 otherwise
 *
 */
static inline uint8_t _is_full_power_req_(void)
{
	uint8_t temp;
	uint16_t v;

	Param_Access(TX_DELAY_FULLPOWER,  (uint8_t*)&(v), 0 );
	v = __ntohs(v);
	if (_u16FullPower_ >= v)
	{
		// go back in full power
		temp = PHY_PMAX_minus_0db;
		Param_Access(TX_POWER, &temp, 1 );
		_u16FullPower_ = 0;
		return 1;
	}
	else
	{
		_u16FullPower_++;
		return 0;
	}
}
/******************************************************************************/

/*! @} */

#ifdef __cplusplus
}
#endif
