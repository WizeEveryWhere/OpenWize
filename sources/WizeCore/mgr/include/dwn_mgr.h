/**
  * @file dwn_mgr.h
  * @brief This file declare functions to use the Download session Manager
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
  * @par 1.0.0 : 2020/09/04[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_dwn_mgr
 * @{
 *
 */

#ifndef _DWN_MGR_H_
#define _DWN_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "ses_common.h"

/******************************************************************************/

/*!
 * @cond INTERNAL
 * @{
 */
#undef RECV_BUFFER_SZ
#define RECV_BUFFER_SZ 210 // L7: 210; L6: 8 ; L2: 38
/*!
 * @}
 * @endcond
 */

/*!
 * @brief This struct defines the download manager internal context.
 */
struct dwn_mgr_ctx_s
{
    uint8_t aRecvBuff[RECV_BUFFER_SZ]
		__attribute__ ((aligned (8))); /**< Buffer that hold the received frame */

    net_msg_t sRecvMsg;          /**< Receive message content */

    // from anndownload
    uint8_t u8ChannelId;         /**< The download channel id */
	uint8_t u8ModulationId;      /**< The download modulation id */
	uint16_t u16BlocksCount;     /**< The total number of block */
    uint32_t u32DaysProg;        /**< The first day (Unix epoch) */
	uint8_t  u8DayRepeat;        /**< The number of time the session is repeated */
	uint8_t  u8DeltaSec;         /**< Period between two block (in second) */

	// Internal variables
	uint8_t u8Pending;           /**< A received block is pending */
    uint8_t u8RxLength;          /**< Listening block window duration (in second) */
    int16_t i16DeltaRxMs;        /**< Offset to listening window (in millisecond) */
	uint16_t u16InitDelayMinMs;  /**<  */

	uint16_t _u16BlocksCount;    /**< Remaining block */
	uint32_t _u32DayNext;        /**< The next day date */
	uint8_t  _u8DayCount;        /**< Remaining day */

	// TODO : Not used
	uint32_t _u32RemainInDay;    /**< NC */
	uint32_t _u32RemainInBlock;  /**< NC */
	uint32_t _u32NextBlkDelay;   /**< NC */
};

void DwnMgr_Setup(struct ses_ctx_s *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _DWN_MGR_H_ */

/*! @} */
