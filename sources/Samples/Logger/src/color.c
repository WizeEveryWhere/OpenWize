
/**
  * @file color.c
  * @brief This file hold the color table...
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
  * @par 1.0.0 : 2020/11/19[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup logger
 * @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "color.h"
#include "logger.h"

/*!
 * @cond INTERNAL
 * @{
 */

#ifdef LOGGER_HAS_COLOR
const char *color_str[LOG_LV_ID_MAX] =
{
	[LOG_LV_ID_QUIET]   = SGR(FGC(WHITE)),
	[LOG_LV_ID_ERR]     = SGR(BOLD";"FGC(RED)),
	[LOG_LV_ID_WRN]     = SGR(FGC(YELLOW)),
	[LOG_LV_ID_INF]     = SGR(FGC(GREEN)),
	[LOG_LV_ID_DBG]     = SGR(FGC(CYAN)),
	[LOG_LV_ID_FRM_IN]  = SGR(FGC(MAGENTA)),
	[LOG_LV_ID_FRM_OUT] = SGR(FGC(MAGENTA)),
};
#endif

/*!
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

/*! @} */

