/**
  * @file time_mgr.h
  * @brief This file implement the time correction related functions
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
  * @par 1.0.0 : 2021/06/28[GBI]
  * Initial version
  *
  *
  */

#ifndef _TIME_MGR_H_
#define _TIME_MGR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/******************************************************************************/
/*!
 * @addtogroup wize_time_mgr
 * @{
 *
 */

/******************************************************************************/
/*!
 * @brief This enum define the time manager return flags
 */
typedef enum
{
	TIME_FLG_NONE          = 0x00, /*!< Nothing */
	// ---
	TIME_FLG_CLOCK_ADJ     = 0x01, /*!< The current clock has been set */
	TIME_FLG_OFFSET_ADJ    = 0x02, /*!< The current clock has been adjusted with offset */
	TIME_FLG_DRIFT_ADJ     = 0x03, /*!< The current clock has been adjusted with drift */
	// ---
	TIME_FLG_CLOCK_CHANGE  = 0x10, /*!< The CLOCK parameter change has been took into account */
	TIME_FLG_OFFSET_CHANGE = 0x20, /*!< The OFFSET parameter change has been took into account */
	TIME_FLG_DRIFT_CHANGE  = 0x40, /*!< The DRIFT parameter change has been took into account */
	// ---
	TIME_FLG_DAY_PASSED    = 0x80,  /*!< A new day event occurs */
	// ---
	// convenient mask
	TIME_FLG_TIME_ADJ      = 0x0F,
	TIME_FLG_TIME_CHANGE   = 0x70,
} time_flg_e;

/*!
 * @brief This struct define the time update context
 */
struct time_upd_s
{
	uint32_t value; //!< Value of currently pending correction
	union {
		uint16_t drift;
		struct {
			uint8_t period; //!< Drift correction period
			int8_t  value;  //!< Drift correction value
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

/*!
 * @brief This struct define the time update internal context
 */
struct time_upd_ctx_s
{
	void *hTask;
	uint32_t u32Epoch;
	uint32_t u32OffsetToUnix;
	uint32_t *pCurEpoch;
	uint16_t *pCurOffset;
	uint16_t *pCurDrift;
	struct time_upd_s *pTimeUpd;
};

uint32_t TimeMgr_Main(struct time_upd_ctx_s *pCtx, uint8_t bNewDay);


/*! @} */

#ifdef __cplusplus
}
#endif
#endif /* _TIME_MGR_H_ */
