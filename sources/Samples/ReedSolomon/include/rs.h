/**
  * @file rs.h
  * @brief This file expose prototype of encode and decode function for the
  * Reed-Solomon error correction code.
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
  * @par 1.0.0 : 2020/01/01[SCO]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup reed_solomon
 * @{
 *
 */
#ifndef _RS_H_
#define _RS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*!
 * @def mm
 * @brief Define the symboles bits number
 */
#define mm  8           /* RS code over GF(2**8) - change to suit */

/*!
 * @def nn
 * @brief Define the total message size (with parity word)
 */
#define nn  255          /* nn=2**mm -1   length of codeword */

/*!
 * @def tt
 * @brief Define the number of error that can be corrected
 */
#define tt  16           /* number of errors that can be corrected */

//#define kk  (nn - PARITY_RS_SIZE) /* kk = nn-2*tt  */

/*!
 * @def kk
 * @brief Define the useable message size
 */
#define kk (223) /* kk = nn-2*tt  */

/*!
 * @def RS_MESSAGE_SZ
 * @brief Define an alias to kk
 */
#define RS_MESSAGE_SZ kk

/*!
 * @def RS_PARITY_SZ
 * @brief Define the parity word size
 */
#define RS_PARITY_SZ (tt*2)

void RS_Init(void);
uint8_t RS_Decode(uint8_t p_Data[RS_MESSAGE_SZ + RS_PARITY_SZ]);
void RS_Encode(uint8_t p_Data[RS_MESSAGE_SZ], uint8_t p_Out[RS_PARITY_SZ]);

// debugging helper function
uint32_t RS_GetMsgSize(void);
uint32_t RS_GetParitySize(void);
const uint8_t* RS_GetGG_ptr(uint32_t *u32_ggSz);
const uint8_t* RS_GetAlphaOf_ptr(uint32_t *u32_alphaOfSz);
const int16_t* RS_GetIndexOf_ptr(uint32_t *u32_indexOfSz);


#ifdef __cplusplus
}
#endif
#endif /* _RS_H_ */

/*! @} */
