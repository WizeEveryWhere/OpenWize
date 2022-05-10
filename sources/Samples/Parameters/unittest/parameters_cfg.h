/*!
  * @file parameters_cfg.h
  * @brief This file was generated from ../../../Config/param_config/Test/DefaultParams_test.xml(Modif. : 2020-02-24 22:45:40.015160900 +0100).
  * 
  *****************************************************************************
  * @copyright 2020, GRDF, Inc.  All rights reserved.
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
  * Generation Date
  * ----------------
  * 24/02/2020 22:45 [neo]
  *
  */


#ifndef _PARAMETERS_ID_H_
#define _PARAMETERS_ID_H_
#ifdef __cplusplus
extern C {
#endif

#include "parameters_def.h"

/******************************************************************************/
#define PARAM_ACCESS_CFG_SZ (0x26)
#define PARAM_DEFAULT_SZ (0x65)

extern const param_s a_ParamAccess[PARAM_ACCESS_CFG_SZ];
extern const uint8_t a_ParamDefault[PARAM_DEFAULT_SZ];

typedef enum {
    PARAM_01 = 0x01, //!< Parameter 01; Range; 8BITS
    PARAM_02 = 0x02, //!< Parameter 02; Range; 16BITS
    PARAM_03 = 0x03, //!< Parameter 03; Range; 32BITS
    PARAM_04 = 0x04, //!< Parameter 04; Range; 64BITS
    PARAM_05 = 0x05, //!< Parameter 05; Enum; 8BITS
    PARAM_06 = 0x06, //!< Parameter 06; Enum; 16BITS
    PARAM_07 = 0x07, //!< Parameter 07; Enum; 32BITS
    PARAM_08 = 0x08, //!< Parameter 08; Enum; 64BITS
    PARAM_09 = 0x09, //!< Parameter 09; Modulo; 8BITS
    PARAM_0A = 0x0A, //!< Parameter 0A; Modulo; 16BITS
    PARAM_0B = 0x0B, //!< Parameter 0B; Modulo; 32BITS
    PARAM_0C = 0x0C, //!< Parameter 0C; Modulo; 64BITS
    PARAM_10 = 0x10, //!< Parameter 10;
    PARAM_11 = 0x11, //!< Parameter 11; loc RO
    PARAM_12 = 0x12, //!< Parameter 12; rem RO
    PARAM_20 = 0x20, //!< Parameter 20; REF_N
    PARAM_21 = 0x21, //!< Parameter 21; ACK
    PARAM_22 = 0x22, //!< Parameter 22; MNT
    PARAM_23 = 0x23, //!< Parameter 23; HGA
    PARAM_24 = 0x24, //!< Parameter 24; UTC
    PARAM_25 = 0x25, //!< Parameter 25;
    LAST_ID = 0x25, //!< Don't remove, it marks the end of table.
}param_ids_e;

/******************************************************************************/
#define PARAM_RESTR_CFG_SZ (0xD)

extern const restr_s a_ParamRestr[PARAM_RESTR_CFG_SZ];

extern const uint8_t a_RANGE_01[2];
extern const uint16_t a_RANGE_02[2];
extern const uint32_t a_RANGE_03[2];
extern const uint64_t a_RANGE_04[2];
extern const uint8_t a_ENUM_05[4];
extern const uint16_t a_ENUM_06[5];
extern const uint32_t a_ENUM_07[6];
extern const uint64_t a_ENUM_08[7];
extern const uint8_t a_MODULO_09[1];
extern const uint16_t a_MODULO_0A[1];
extern const uint32_t a_MODULO_0B[1];
extern const uint64_t a_MODULO_0C[1];

#ifdef __cplusplus
}
#endif
#endif /* _PARAMETERS_ID_H_ */
