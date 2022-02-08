/*!
  * @file parameters_cfg.c
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

#include "parameters_def.h"
#include "parameters_cfg.h"


/******************************************************************************/
const param_s a_ParamAccess[0x26] = {
    INIT_ACCESS_TABLE(0x0, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x01, RW, RW, IMM, REF_Y, 1, 0, 0x01),
    INIT_ACCESS_TABLE(0x02, RW, RW, IMM, REF_Y, 2, 1, 0x02),
    INIT_ACCESS_TABLE(0x03, RW, RW, IMM, REF_Y, 4, 3, 0x03),
    INIT_ACCESS_TABLE(0x04, RW, RW, IMM, REF_Y, 8, 7, 0x04),
    INIT_ACCESS_TABLE(0x05, RW, RW, IMM, REF_Y, 1, 15, 0x05),
    INIT_ACCESS_TABLE(0x06, RW, RW, IMM, REF_Y, 2, 16, 0x06),
    INIT_ACCESS_TABLE(0x07, RW, RW, IMM, REF_Y, 4, 18, 0x07),
    INIT_ACCESS_TABLE(0x08, RW, RW, IMM, REF_Y, 8, 22, 0x08),
    INIT_ACCESS_TABLE(0x09, RW, RW, IMM, REF_Y, 1, 30, 0x05),
    INIT_ACCESS_TABLE(0x0A, RW, RW, IMM, REF_Y, 2, 31, 0x06),
    INIT_ACCESS_TABLE(0x0B, RW, RW, IMM, REF_Y, 4, 33, 0x07),
    INIT_ACCESS_TABLE(0x0C, RW, RW, IMM, REF_Y, 8, 37, 0x08),
    INIT_ACCESS_TABLE(0xD, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0xE, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0xF, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x10, RW, RW, IMM, REF_Y, 6, 45, 0x0),
    INIT_ACCESS_TABLE(0x11, RO, RW, IMM, REF_Y, 6, 51, 0x0),
    INIT_ACCESS_TABLE(0x12, RW, RO, IMM, REF_Y, 6, 57, 0x00),
    INIT_ACCESS_TABLE(0x13, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x14, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x15, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x16, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x17, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x18, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x19, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x1A, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x1B, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x1C, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x1D, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x1E, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x1F, NA, NA, IMM, REF_N, 0, 0, 0x0),
    INIT_ACCESS_TABLE(0x20, RW, RW, IMM, REF_N, 6, 63, 0x0),
    INIT_ACCESS_TABLE(0x21, RO, RW, ACK, REF_Y, 6, 69, 0x0),
    INIT_ACCESS_TABLE(0x22, RW, RO, MNT, REF_Y, 6, 75, 0x0),
    INIT_ACCESS_TABLE(0x23, RO, RW, HGA, REF_Y, 6, 81, 0x0),
    INIT_ACCESS_TABLE(0x24, RW, RO, UTC, REF_Y, 6, 87, 0x0),
    INIT_ACCESS_TABLE(0x25, NA, NA, IMM, REF_Y, 8, 93, 0x0),
};

/******************************************************************************/
const restr_s a_ParamRestr[0xD] = {
    INIT_RESTR_TABLE(0x01, RESTR_RANGE, RESTR_8BITS, 2, a_RANGE_01),
    INIT_RESTR_TABLE(0x02, RESTR_RANGE, RESTR_16BITS, 2, a_RANGE_02),
    INIT_RESTR_TABLE(0x03, RESTR_RANGE, RESTR_32BITS, 2, a_RANGE_03),
    INIT_RESTR_TABLE(0x04, RESTR_RANGE, RESTR_64BITS, 2, a_RANGE_04),
    INIT_RESTR_TABLE(0x05, RESTR_ENUM, RESTR_8BITS, 4, a_ENUM_05),
    INIT_RESTR_TABLE(0x06, RESTR_ENUM, RESTR_16BITS, 5, a_ENUM_06),
    INIT_RESTR_TABLE(0x07, RESTR_ENUM, RESTR_32BITS, 6, a_ENUM_07),
    INIT_RESTR_TABLE(0x08, RESTR_ENUM, RESTR_64BITS, 7, a_ENUM_08),
    INIT_RESTR_TABLE(0x09, RESTR_MODULO, RESTR_8BITS, 1, a_MODULO_09),
    INIT_RESTR_TABLE(0x0A, RESTR_MODULO, RESTR_16BITS, 1, a_MODULO_0A),
    INIT_RESTR_TABLE(0x0B, RESTR_MODULO, RESTR_32BITS, 1, a_MODULO_0B),
    INIT_RESTR_TABLE(0x0C, RESTR_MODULO, RESTR_64BITS, 1, a_MODULO_0C),
};

