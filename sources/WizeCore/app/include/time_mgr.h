/**
  * @file time_mgr.h
  * @brief This file implement the time correction related functions
  * 
  *****************************************************************************
  * @Copyright 2021, GRDF, Inc.  All rights reserved.
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
  * 1.0.0 : 2021/06/28[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup WizeCore
 * @{
 *
 */
#ifndef _TIME_MGR_H_
#define _TIME_MGR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "time_evt.h"
/******************************************************************************/

/*!
 * @brief This struct define the time update context
 */
struct time_upd_s
{
	uint32_t value; //!< Value of currently pending correction
	union {
		uint16_t drift;
		struct {
			int8_t  value;  //!< Drift correction value
			uint8_t period; //!< Drift correction period
		};
	}drift_;

	uint8_t days_cnt;    //!< Number of remaining days for the Drift correction
	union {
		uint8_t state; //!< Currently pending correction
		struct
		{
			uint8_t clock_init:1;  //!< Clock Time has been initialized
			uint8_t epoch_pend:1;  //!< Epoch correction is pending
			uint8_t offset_pend:1; //!< Offset correction is pending
		} ;
	}state_;
} ;

extern void _time_wakeup_enable(void);
extern void _time_wakeup_reload(void);
extern void _time_wakeup_force(void);
extern void _time_update_set_handler(pfTimeEvt_HandlerCB_t const pfCb);

void TimeMgr_Setup(struct time_upd_s *pTimeUpdCtx);

#ifdef __cplusplus
}
#endif
#endif /* _TIME_MGR_H_ */

/*! @} */
