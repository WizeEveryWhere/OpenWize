/**
  * @file: ses_common.c
  * @brief This file contains common session definitions
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
  * @par 1.0.0 : 2021/03/16[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_mgr
 * @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "ses_common.h"

/******************************************************************************/
/*!
 * @brief This define the session (debug) state string
 */
const char * const _ses_state_str_[] =
{
	[SES_STATE_DISABLE]            = "DISABLE",
	[SES_STATE_IDLE]               = "IDLE",
	[SES_STATE_SENDING]            = "SENDING",
	[SES_STATE_LISTENING]          = "LISTENING",
	[SES_STATE_WAITING]            = "WAITING",
	[SES_STATE_WAITING_RX_DELAY]   = "WAIT_RX_DELAY",
	[SES_STATE_WAITING_TX_DELAY]   = "WAIT_TX_DELAY",
};

#ifdef __cplusplus
}
#endif

/*! @} */
