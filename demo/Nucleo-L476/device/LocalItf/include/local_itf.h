/**
  * @file: local_itf.h
  * @brief: // TODO This file ...
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
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
  * 1.0.0 : 2021/11/04[GBI]
  * Initial version
  *
  *
  */
#ifndef _LOCAL_ITF_H_
#define _LOCAL_ITF_H_
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
#endif /* _LOCAL_ITF_H_ */
