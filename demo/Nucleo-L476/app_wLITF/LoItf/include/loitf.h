/**
  * @file: loitf.h
  * @brief: // TODO This file ...
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
  * @par 1.0.0 : 2022/08/16 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup app_wLITF
  * @{
 */

#ifndef _LOITF_H_
#define _LOITF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "common.h"

typedef struct itf_dev_s
{
	void *pDev;
    pfEvtCb_t pfEvent;
}itf_dev_t;



void LoItf_Setup(void);

#ifdef __cplusplus
}
#endif
#endif /* _LOITF_H_ */

/*! @} */
