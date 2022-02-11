/**
  * @file: crc_sw.h
  * @brief: This file expose the prototype function to compute and check a CRC.
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
  * 1.0.0 : 2020/01/01[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup Crc_sw
 * @{
 */

#ifndef _CRC_SW_H_
#define _CRC_SW_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t CRC_Check(uint16_t u16_CrcA, uint16_t u16_CrcB);
uint8_t CRC_Compute(uint8_t * p_Buf, uint8_t u8_Sz, uint16_t *p_Crc);

#ifdef __cplusplus
}
#endif
#endif /* _CRC_SW_H_ */

/*! @} */
/*! @} */
/*! @} */
