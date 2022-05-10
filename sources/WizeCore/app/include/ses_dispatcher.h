/**
  * @file: ses_dispatcher.h
  * @brief This file define the session dispatcher structures
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
  * @par 1.0.0 : 2020/11/22[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_app
 * @{
 *
 */
#ifndef _SES_DISPATCHER_H_
#define _SES_DISPATCHER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "ses_common.h"

#include "inst_mgr.h"
#include "adm_mgr.h"
#include "dwn_mgr.h"

#include "inst_internal.h"
#include "adm_internal.h"

/******************************************************************************/

#define SES_MGR_EVT_POS 24
#define SES_MGR_FLG_POS 16

/*!
 * @brief This enumeration define
 */
typedef enum
{
	SES_MGR_EVT_MSK        = 0xFF000000,
	SES_MGR_INST_EVT_OPEN  = (SES_INST << SES_MGR_EVT_POS) | SES_EVT_OPEN,
	SES_MGR_INST_EVT_CLOSE = (SES_INST << SES_MGR_EVT_POS) | SES_EVT_CLOSE,

	SES_MGR_ADM_EVT_OPEN   = (SES_ADM << SES_MGR_EVT_POS) | SES_EVT_OPEN,
	SES_MGR_ADM_EVT_CLOSE  = (SES_ADM << SES_MGR_EVT_POS) | SES_EVT_CLOSE,
} ses_mgr_evt_e;

/*!
 * @brief This enumeration define
 */
typedef enum
{
	SES_MGR_FLG_NONE    = 0x0,
	SES_MGR_FLG_SUCCESS = 0x1,
	SES_MGR_FLG_FAILED  = 0x2,
	SES_MGR_FLG_REQUEST = 0x3,

	SES_MGR_FLG_MSK          = 0x00FF0000,
	SES_MGR_ADM_FLG_SUCCES   = (SES_ADM << SES_MGR_FLG_POS)  | SES_MGR_FLG_SUCCESS,
	SES_MGR_ADM_FLG_FAILED   = (SES_ADM << SES_MGR_FLG_POS)  | SES_MGR_FLG_FAILED,
	SES_MGR_ADM_FLG_REQUEST  = (SES_ADM << SES_MGR_FLG_POS)  | SES_MGR_FLG_REQUEST,

	SES_MGR_INST_FLG_SUCCESS = (SES_INST << SES_MGR_FLG_POS) | SES_MGR_FLG_SUCCESS,
	SES_MGR_INST_FLG_FAILED  = (SES_INST << SES_MGR_FLG_POS) | SES_MGR_FLG_FAILED,
} ses_mgr_flg_e;

/*!
 * @brief This enumeration define
 */
typedef enum
{
	GLO_FLG_NONE              = 0x00000000,

	// -------------------------------------------------------------------------
	GLO_FLG_SEND_RECV_MSK     = 0x000000FF,
	// ----
	GLO_FLG_CMD_RECV_READ     = 0x00000001,
	GLO_FLG_CMD_RECV_WRITE    = 0x00000002,
	GLO_FLG_CMD_RECV_KEY      = 0x00000003,
	GLO_FLG_CMD_RECV_EXEC     = 0x00000004,
	GLO_FLG_CMD_RECV_ANN      = 0x00000005,

	GLO_FLG_CMD_RECV          = 0x00000007,
	GLO_FLG_PONG_RECV         = 0x00000008,

	GLO_FLG_BLK_RECV          = 0x00080010,

	GLO_FLG_DATA_SENT         = 0x00000020, // SES_FLG_DATA_SENT >> 4
	GLO_FLG_RSP_SENT          = 0x00000040, // SES_FLG_RSP_SENT >> 4
	GLO_FLG_PING_SENT         = 0x00000080, // SES_FLG_PING_SENT >> 4

	// -------------------------------------------------------------------------
	GLO_FLG_TIME_MSK          = 0x00000F00,
	// ----
	GLO_FLG_FINE_ADJUST       = 0x00000100,
	GLO_FLG_COARSE_ADJUST     = 0x00000200,
	GLO_FLG_DAY_PASSED        = 0x00000400,
	GLO_FLG_DAYLIGHT_CHG      = 0x00000800,

	// -------------------------------------------------------------------------
	GLO_FLG_SESSION_MSK       = 0x00FFF000,
	// ----
	GLO_FLG_ADM_START         = 0x00001000,
	GLO_FLG_ADM_COMPLETE      = 0x00002000,
	GLO_FLG_ADM_ERROR         = 0x00004000,
	GLO_FLG_FULL_POWER        = 0x00008000,

	GLO_FLG_INST_START        = 0x00010000,
	GLO_FLG_INST_COMPLETE     = 0x00020000,
	GLO_FLG_INST_ERROR        = 0x00040000,
	GLO_FLG_PERIODIC_INST     = 0x00080000,

	GLO_FLG_DWN_START         = 0x00100000,
	GLO_FLG_DWN_COMPLETE      = 0x00200000,
	GLO_FLG_DWN_ERROR         = 0x00400000,
	GLO_FLG_DWN_CORRUPTED     = 0x00800000,

	// -------------------------------------------------------------------------
	//GLO_FLG_SESSION_MSK       = 0x00FFF000,
	// ----
	GLO_FLG_ADM_ACTIVE        = 0x01000000,
	GLO_FLG_ADM_TIMEOUT       = 0x02000000,

	GLO_FLG_INST_ACTIVE       = 0x04000000,
	GLO_FLG_INST_TIMEOUT      = 0x08000000,

	GLO_FLG_DWN_ACTIVE        = 0x10000000,
	GLO_FLG_DWN_TIMEOUT       = 0x20000000,
} glo_flg_e;

/*!
 * @brief This enumeration define
 */
typedef enum
{
	SES_DISP_STATE_DISABLE,
	SES_DISP_STATE_ENABLE,
} ses_disp_state_e;

/*!
 * @brief This enumeration define
 */
typedef enum
{
	SES_ADM_CMD_PEND  = 0x01,
	SES_ADM_RSP_PEND  = 0x02,

	SES_ADM_SES_PEND  = 0x10,
	SES_INST_SES_PEND = 0x20,
	SES_DWN_SES_PEND  = 0x40,
} ses_disp_pending_e;

/*!
 * @brief This struct defines the session dispatcher context.
 */
struct ses_disp_ctx_s
{
	void *hTask;
	void *hMutex;
	void *hEvents;
	struct ses_ctx_s *pActive;
	struct ses_ctx_s sSesCtx[SES_NB];

	struct inst_mgr_ctx_s sInstMgrCtx;
	struct adm_mgr_ctx_s  sAdmMgrCtx;
	struct dwn_mgr_ctx_s  sDwnMgrCtx;

	uint8_t bPendAction;

	ses_type_t eActiveId;
	ses_type_t eReqId;

	ses_disp_state_e eState;
	struct ping_reply_ctx_s sPingReplyCtx;

	uint16_t u16PeriodInst; // in days (from param EXECPING_PERIOD convert from month to days)
	uint16_t u16FullPower; // in days

};

void SesDisp_Setup(struct ses_disp_ctx_s *pCtx);
void SesDisp_Init(struct ses_disp_ctx_s *pCtx, uint8_t bEnable);

static inline uint32_t _get_pos(uint32_t ulFlg)
{
	register uint32_t flg;
	if (ulFlg)
	{
		flg = ulFlg >> 4;
		if (!flg)
		{
			flg = ulFlg;
		}
		return (32 -  __builtin_clzl (flg));
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
#endif /* _SES_DISPATCHER_H_ */

/*! @} */
