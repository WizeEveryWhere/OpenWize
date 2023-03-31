/**
  * @file wize_app.c
  * @brief This file implement some convenient functions to deal with the Wize
  *        application layer.
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

#include "parameters.h"
#include "parameters_lan_ids.h"

/*!
 * @addtogroup wize_app
 * @{
 *
 */

/******************************************************************************/
/*!
 * @brief This variable is used to hold the "periodic install" counter in day unit.
 *
 * @details It is initialize at 0 and incremented (in "WizeApp_Time" function)
 *          until it reach EXECPING_PERIOD (converted from month into days).
 *          Depending on the low power mode, it could be required to save it in
 *          non-volatile memory or backup-register.
 */
uint16_t gu16PeriodInstCnt;

/*!
 * @brief This variable is used to hold the "full power" counter in day unit.
 *
 * @details It is initialize at 0 and incremented (in "WizeApp_Time" function)
 *          until it reach TX_DELAY_FULLPOWER or if any ADM CMD is received.
 *          Depending on the low power mode, it could be required to save it in
 *          non-volatile memory or backup-register.
 */
uint16_t gu16FullPowerCnt;

/*!
 * @brief This structure is used to hold the installation context information.
 */
struct inst_mgr_ctx_s sInstCtx;

/*!
 * @brief This structure is used to hold the administration context information.
 *
 * @details Depending on the low power mode, some part should be saved in non-
 *          volatile memory or backup-register.
 *          Especially : sDataMsg.u16Id
 *          Optionally : sCmdMsg.u16Id, sRspMsg.u16Id, aRecvBuff, aSendBuff
 *
 */
// Must be saved (part only)
struct adm_mgr_ctx_s sAdmCtx;

/*!
 * @brief This structure is used to hold the download context information.
 */
struct dwn_mgr_ctx_s sDwnCtx;

/*!
 * @brief This structure is used to hold the time management context information.
 *
 * @details Depending on the low power mode, it could be required to save it in
 *          non-volatile memory or backup-register.
 */
struct time_upd_s sTimeUpdCtx;

/*!
 * @brief This structure is used to hold the temporary installation data.
 */
struct ping_reply_ctx_s sPingReply;

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */
static uint32_t _u32InitMask_;
static uint8_t  _bPendAction_;

static inline uint8_t _is_periodic_inst_req_(void);
static inline uint8_t _is_full_power_req_(void);

/*!
 * @}
 * @endcond
 */

//void onKeyA() __attribute__((weak, alias("onKeyBlank")));

/******************************************************************************/
/*!
 * @brief This is a "weak" function intend to implement the treatment of one
 *        FW block. It will be called by "WizeApp_Common" when a download block
 *        will be received. Its content is user defined.
 *
 * @param [in] u16Id Identify the block number
 * @param [in] pData Pointer on the data block of fixed 210 bytes size.
 *
 * @return 0
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
 * @brief This is a "weak" function intend to implement the checking of the FW
 *        related parameters. It will be called by "WizeApp_Common" when an
 *        administration "ANN_DOWNLOAD" command will be received. Its content
 *        is user defined.
 *
 * @details This function SHOULD call, (depending on use case) :
 *              - "AdmInt_AnnCheckIntFW" for internal FW update request
 *              - 'user defined check ann. external FW' otherwise
 *          Then, this function HAS TO call "WizeApp_AnnReady" function to notify
 *          the result.
 *
 *          For external FW, parameters to check could be  :
 *          - Check Blocks Count             : L7BlocksCount
 *          - Check HW version               : L7DcHwId
 *          - Check Initial SW version       : L7SwVersionIni
 *          - Check Target SW version        : L7SwVersionTarget
 *
 * @param [in] pAnn A pointer on ANN_DOWNLOAD administration message
 *
 * @return 0
 *
 */
__attribute__((weak))
uint8_t WizeApp_AnnCheckFwInfo(admin_cmd_anndownload_t *pAnn)
{
	(void)pAnn;
	/* Check ANNDOWNLOAD to update FW for an external device
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
	 * - Notify ADM that response is ready (call "WizeApp_AnnReady")
	 *
	 * Check ANNDOWNLOAD to update FW internally
	 * - Call "AdmInt_AnnCheckIntFW" function.
	 * - call "WizeApp_AnnReady" function.
	 *
	 */
	return 0;
}

/******************************************************************************/

/*!
 * @brief This function initialize the related global variables
 */
void WizeApp_Init(void)
{
	gu16PeriodInstCnt = 0;
	gu16FullPowerCnt = 0;
	_bPendAction_ = 0;
	_u32InitMask_ = 0xFFFFFFFF;
}

/*!
 * @brief Convenient function that initialize an "sPingReply" structure and
 *        call the "WizeApi_ExecPing"function.
 *
 * @return (see WizeApi_ExecPing)
 *
 */
//inline
wize_api_ret_e WizeApp_Install(void)
{
	// Start Install session
	inst_ping_t sPing = InstInt_Init(&sPingReply);
	return WizeApi_ExecPing((uint8_t*)&sPing, sizeof(inst_ping_t));
}

/*!
 * @brief Convenient function that call "WizeApi_Send" for standard frame.
 *
 * @param [in] pData  Pointer on raw data to send
 * @param [in] u8Size Number of byte to send
 *
 * @return (see WizeApi_Send)
 *
 */
inline
wize_api_ret_e WizeApp_Send(uint8_t *pData, uint8_t u8Size)
{
	// Start ADM Data session
	return WizeApi_Send(pData, u8Size, APP_DATA);
}

/*!
 * @brief Convenient function that call "WizeApi_Send" for priority frame.
 *
 * @param [in] pData  Pointer on raw data to send
 * @param [in] u8Size Number of byte to send
 *
 * @return (see WizeApi_Send)
 *
 */
inline
wize_api_ret_e WizeApp_Alarm(uint8_t *pData, uint8_t u8Size)
{
	// Start ADM Data Prio session
	return WizeApi_Send(pData, u8Size, APP_DATA_PRIO);
}

/******************************************************************************/
/*!
 * @brief This function notify the result of a previously received ANN_DOWNLOAD
 *        command.
 *
 * @details From the ANN_DOWNLOAD message, two kind of parameters :
 *          - Session Related, are already checked in WizeApp_Common function.
 *          - Firmware Related, if it target internal FW, the verification is
 *          done by "AdmInt_AnnCheckIntFW" function, otherwise (external FW)
 *          user has to implement its own. In each case (internal or external
 *          FW), the user application have to call the "FW checking function"
 *          then "WizeApp_AnnReady" with the result.
 *
 *          The input "eErrCode" should belong to administration layer error code
 *          (see admin_ann_err_code_e) and should one of :
 *          - ANN_INCORRECT_HW_VER :
 *              - u8ErrorParam = LSB of current HW version.
 *          - ANN_INCORRECT_INI_SW_VER or ANN_TGT_SW_VER or ANN_TGT_VER_DWL :
 *              - u8ErrorParam = LSB of current SW version.
 *          - ANN_ILLEGAL_VALUE:
 *              - u8ErrorParam = one of admin_ann_param_id_e
 *          - ANN_UPD_IN_PROGRESS:
 *              - u8ErrorParam = TBD (Local vs internal vs external)
 *
 * @param [in] eErrCode     The result error from FW parameters checking.
 * @param [in] u8ErrorParam Erroneous Parameter ID (if any)
 *
 */
void WizeApp_AnnReady(uint8_t eErrCode, uint8_t u8ErrorParam)
{
	admin_rsp_err_t *pRsp;
	if ( _bPendAction_ & WIZEAPP_ADM_RSP_PEND)
	{
		do {
			pRsp = (admin_rsp_err_t *)(sAdmCtx.aSendBuff);
			if ( ( pRsp->L7ResponseId != ADM_ANNDOWNLOAD ) ||
				 ( pRsp->L7ResponseId != sAdmCtx.aRecvBuff[0] ) )
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

/*!
 * @brief This function treat the "most important" application layer flags. Treated
 *        flags are : SES_FLG_CMD_RECV, SES_FLG_RSP_SENT, SES_FLG_PONG_RECV,
 *        SES_FLG_BLK_RECV (see ses_flg_e for details).
 *        This function should be called on each event from the session manager.
 *
 * @retval ADM CMD id when a RSP to previously received CMD is sent (see admin_cmd_id_e)
 *         0 otherwise
 */
uint32_t WizeApp_Common(uint32_t ulEvent)
{
	uint32_t ret = 0;

	ulEvent &= _u32InitMask_;

	// lock

	/*------------------------------------------------------------------------*/
	// ADM back flag
	if (ulEvent & SES_FLG_CMD_RECV)
	{
		// treat CMD and prepare RSP
		uint8_t rsp_available = AdmInt_PreCmd( &(sAdmCtx.sCmdMsg), &(sAdmCtx.sRspMsg) );

		// response not yet available (EXECPING or ANNDOWN external FW case)
		if ( rsp_available == 0)
		{
			_bPendAction_ |= WIZEAPP_ADM_RSP_PEND;
			switch(sAdmCtx.aRecvBuff[0])
			{
				case ADM_EXECINSTPING:
					// Start Install session
					WizeApp_Install();
					break;
				case ADM_ANNDOWNLOAD:
					WizeApp_AnnCheckFwInfo( (admin_cmd_anndownload_t*)(sAdmCtx.aRecvBuff));
					break;
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
		gu16FullPowerCnt = 0;
	}

	if (ulEvent & SES_FLG_RSP_SENT)
	{
		// action must be executed
		if (_bPendAction_ & WIZEAPP_ADM_CMD_PEND)
		{
			// do action and return the last CMD
			ret |= AdmInt_PostCmd( &(sAdmCtx.sCmdMsg), &(sAdmCtx.sRspMsg) );
			_bPendAction_ &= (~WIZEAPP_ADM_CMD_PEND);
		}
	}

	/*
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
	*/

	/*------------------------------------------------------------------------*/
	// INST back flag
	if (ulEvent & SES_FLG_PONG_RECV)
	{
		inst_pong_t *pPong = (inst_pong_t *)(sInstCtx.aRecvBuff);
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
			, sInstCtx.sRspMsg.u8Rssi
			);
		// treat the received message that should be store in sRspMsg
		InstInt_Add(&sPingReply, &(sInstCtx.sRspMsg) );
		// signal that PONG msg has been treated, buffer is ready
		WizeApi_Notify(SES_EVT_INST_READY);
	}

	if (ulEvent & SES_FLG_INST_COMPLETE)
	{
		if ( !(ulEvent & SES_FLG_INST_ERROR) )
		{
			InstInt_End( &sPingReply );
			if ( _bPendAction_ & WIZEAPP_ADM_RSP_PEND)
			{
				if (sAdmCtx.aRecvBuff[0] == ADM_EXECINSTPING)
				{
					_bPendAction_ &= (~WIZEAPP_ADM_RSP_PEND);
					if ( AdmInt_PreCmd( &(sAdmCtx.sCmdMsg), &(sAdmCtx.sRspMsg) ) == 2)
					{
						// signal that RSP is ready
						WizeApi_Notify(SES_EVT_ADM_READY);
					}
				}
				ret = ADM_EXECINSTPING;
			}
			// Install session is complete without error, so clear the "periodic install" counter
			gu16PeriodInstCnt = 0;
		}
	}
	/*
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
	*/

	/*------------------------------------------------------------------------*/
	// DWN back flag

	if (ulEvent & SES_FLG_BLK_RECV)
	{
		WizeApp_OnDwnBlkRecv( sDwnCtx.sRecvMsg.u16Id, sDwnCtx.aRecvBuff );
		WizeApi_Notify(SES_EVT_DWN_READY);
	}
	/*
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
	*/

	/*------------------------------------------------------------------------*/
	// unlock
	return ret;
}

/*!
 * @brief This function check if "Periodic Install" and/or "Back to Full Power"
 *        are required. This should be called every time one day pass.
 *
 * @retval wizeapp_info_e::WIZEAPP_INFO_NONE if none of them are required
 *         wizeapp_info_e::WIZEAPP_INFO_FULL_POWER if a change to full power is required
 *         wizeapp_info_e::WIZEAPP_INFO_PERIO_INST if a periodic install is required
 */
uint32_t WizeApp_Time(void)
{
	uint32_t ret = WIZEAPP_INFO_NONE;
	/*------------------------------------------------------------------------*/
	// On Time day passed flag
	if ( _is_full_power_req_() )
	{
		// Back to Full power is request
		ret |= WIZEAPP_INFO_FULL_POWER;
	}
	if ( _is_periodic_inst_req_() )
	{
		// Periodic install is required
		ret |= WIZEAPP_INFO_PERIO_INST;
	}
	/*------------------------------------------------------------------------*/
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
	if (gu16PeriodInstCnt >= v)
	{
		// request for periodic install
		return 1;
	}
	else
	{
		gu16PeriodInstCnt++;
		return 0;
	}
}

/*!
 * @static
 * @brief This function update "Full Power" counter
 *
 * @return 1 if "full power" is required, 0 otherwise
 *
 */
static inline uint8_t _is_full_power_req_(void)
{
	uint8_t temp;
	uint16_t v;

	Param_Access(TX_DELAY_FULLPOWER,  (uint8_t*)&(v), 0 );
	v = __ntohs(v);
	if (gu16FullPowerCnt >= v)
	{
		// go back in full power
		gu16FullPowerCnt = 0;
		return 1;
	}
	else
	{
		gu16FullPowerCnt++;
		return 0;
	}
}
/******************************************************************************/

/*! @} */

#ifdef __cplusplus
}
#endif
