/**
  * @file app_entry.h
  * @brief Header file for app_entry
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
  * @par 1.0.0 : 2019/11/20 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup app_wLITF
 * @{
 */
#ifndef _APP_ENTRY_H_
#define _APP_ENTRY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "FreeRTOS.h"
#include "task.h"

#include "bsp.h"

#include "parameters_cfg.h"
#include "parameters.h"
#include "crypto.h"

#include "wize_banner.h"
#include "wize_api.h"

#include "rtos_macro.h"

#ifdef __cplusplus
}
#endif

#endif /* _APP_ENTRY_H_ */

/*! @} */
