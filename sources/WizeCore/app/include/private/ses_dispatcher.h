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

/******************************************************************************/
/*!
 * @brief This enumeration define
 */
typedef enum
{
	SES_DISP_STATE_DISABLE,
	SES_DISP_STATE_ENABLE,
} ses_disp_state_e;

/*!
 * @brief This struct defines the session dispatcher context.
 */
struct ses_disp_ctx_s
{
	void *hTask;
	struct ses_ctx_s sSesCtx[SES_NB];
	struct ses_ctx_s *pActive;

	ses_disp_state_e eState;

	uint32_t u32InstDurationMs;
	uint32_t u32DataDurationMs;
	uint32_t u32CmdDurationMs;
	uint32_t u32RspDurationMs;

	uint32_t forbidden_msk;
	uint8_t u8ActiveSes;
};

void SesDisp_Setup(struct ses_disp_ctx_s *pCtx);
void SesDisp_Init(struct ses_disp_ctx_s *pCtx, uint8_t bEnable);
uint32_t SesDisp_Fsm(struct ses_disp_ctx_s *pCtx, uint32_t u32Event);

#ifdef __cplusplus
}
#endif
#endif /* _SES_DISPATCHER_H_ */

/*! @} */
