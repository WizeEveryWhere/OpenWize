/**
  * @file parameters_def.h
  * @brief This file define structure and enum to deal with parameters.
  * 
  *****************************************************************************
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
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2019/11/20 00:09:40 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup Parameters
 * @{
 */

#ifndef _PARAMETERS_DEF_H_
#define _PARAMETERS_DEF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/*!
 * @brief This enum defines the parameter access type.
 */
typedef enum {
    NA = 0b00, //!< No Access
    RO = 0b01, //!< Read only
    WO = 0b10, //!< Write only
    RW = 0b11, //!< Read/Write
}param_access_e;

/*!
 * @brief This enum defines the parameter update effectiveness.
 */
typedef enum {
    IMM = 0b000, //!< Immediate
    ACK = 0b001, //!< After the write request acknowledge
    MNT = 0b010, //!< At the next maintenance period
    HGA = 0b011, //!< At the next Gas Hour
    UTC = 0b100, //!< At the next 0h00 UTC
}param_effective_e;

/*!
 * @brief This enum defines if the parameter is saved or not into the referential.
 */
typedef enum {
	REF_N = 0b0, //!< Not saved into the referential
    REF_Y = 0b1, //!< Saved into the referential
}param_ref_e;

/*!
 * @def ACCESS_MSK
 * @brief This define the access mask
 */
#define ACCESS_MSK 0b1111

/*!
 * @def REF_ACCESS_POS
 * @brief This define the referenced position
 */
#define REF_ACCESS_POS 7
/*!
 * @def REF_ACCESS_SZ
 * @brief This define the referenced size
 */
#define REF_ACCESS_SZ 1
/*!
 * @def REF_ACCESS_MSK
 * @brief This define the referenced mask
 */
#define REF_ACCESS_MSK 0b1

/*!
 * @def EFF_ACCESS_POS
 * @brief This define the effectiveness position
 */
#define EFF_ACCESS_POS 4
/*!
 * @def EFF_ACCESS_SZ
 * @brief This define the effectiveness size
 */
#define EFF_ACCESS_SZ 3
/*!
 * @def EFF_ACCESS_MSK
 * @brief This define the effectiveness mask
 */
#define EFF_ACCESS_MSK 0b111

/*!
 * @def LOC_ACCESS_POS
 * @brief This define the local access position
 */
#define LOC_ACCESS_POS 2
/*!
 * @def LOC_ACCESS_SZ
 * @brief This define the local access size
 */
#define LOC_ACCESS_SZ 2
/*!
 * @def LOC_ACCESS_MSK
 * @brief This define the local access mask
 */
#define LOC_ACCESS_MSK 0b11

/*!
 * @def REM_ACCESS_POS
 * @brief This define the remote access position
 */
#define REM_ACCESS_POS 0
/*!
 * @def REM_ACCESS_SZ
 * @brief This define the remote access size
 */
#define REM_ACCESS_SZ 2
/*!
 * @def REM_ACCESS_MSK
 * @brief This define the remote access mask
 */
#define REM_ACCESS_MSK 0b11

/*!
 * @brief This structure defines the parameter properties.
 */
typedef struct __attribute__((__packed__)){
#ifdef PARAM_USE_ADDRESS
	uint32_t  u32_address; //!< Parameter variable address
#else // PARAM_USE_OFFSET
    uint16_t  u16_offset; //!< Parameter variable offset
#endif
	uint8_t   u8_size;    //!< Parameter size (in bytes)
	uint8_t   u8_access;  /*!< Access 
						     @li b7 : referenced (@link param_ref_e @endlink).
						     @li b6-b4 : effectiveness (@link param_effective_e @endlink).
						     @li b3-b2 : local Read/Write (@link param_access_e @endlink).
						     @li b1-b0 : remote Read/Write (@link param_access_e @endlink).
						   */
	uint8_t   u8_restId;   //!< Restriction table id (0x00 means no restriction)
}param_s;

/*!
 * @def RESTR_MSK
 * @brief This define the restriction mask
 */
#define RESTR_MSK 0b01110111
/*!
 * @def TYPE_RESTR_POS
 * @brief This define the restriction type position
 */
#define TYPE_RESTR_POS 0
/*!
 * @def TYPE_RESTR_SZ
 * @brief This define the number of bits used for the restriction type
 */
#define TYPE_RESTR_SZ 3
/*!
 * @def TYPE_RESTR_MSK
 * @brief This define the restriction type mask
 */
#define TYPE_RESTR_MSK 0b0111
/*!
 * @def SZ_RESTR_POS
 * @brief This define the restriction size position
 */
#define SZ_RESTR_POS 4
/*!
 * @def SZ_RESTR_SZ
 * @briefThis define the number of bits used for the restriction size
 */
#define SZ_RESTR_SZ 3
/*!
 * @def SZ_RESTR_MSK
 * @brief This define the restriction size mask
 */
#define SZ_RESTR_MSK 0b0111

/*!
 * @def GET_RESTR_TYPE
 * @brief Get the restr type
 */
#define GET_RESTR_TYPE(restr) ((restr >> TYPE_RESTR_POS) & (TYPE_RESTR_MSK))

/*!
 * @def GET_RESTR_SZ
 * @brief Get the restr size
 */
#define GET_RESTR_SZ(restr)   ((restr >> SZ_RESTR_POS) & (SZ_RESTR_MSK))

/*!
 * @def CHECK_MODULO
 * @brief Check that given value conform to the modulo operator.
 */
#define CHECK_MODULO(val, mod)     ( (val%mod)?(0):(1) )

/*!
 * @def CHECK_RANGE
 * @brief Check that given value conform to the range operator.
 */
#define CHECK_RANGE(val, min, max) ( ( (val >= min) && (val <= max) )?(1):(0) )

/*!
 * @def CHECK_ENUM_VAL
 * @brief Check that given value conform to the enum value.
 */
#define CHECK_ENUM_VAL(val, enumval) ( (val == enumval)?(1):(0) )

/*!
 * @brief This enum defines the parameter restriction type.
 */
typedef enum {
	RESTR_MODULO = 0b001, //!< Restriction on modulo
	RESTR_RANGE  = 0b010, //!< Restriction on range
	RESTR_ENUM   = 0b011, //!< Restriction on enum list
}restr_type_e;

/*!
 * @brief This enum defines the size on which restriction is applied.
 */
typedef enum {
	RESTR_8BITS  = 0b001, //!< Restriction values are on 8 bits
	RESTR_16BITS = 0b010, //!< Restriction values are on 16 bits
	RESTR_32BITS = 0b011, //!< Restriction values are on 32 bits
	RESTR_64BITS = 0b100, //!< Restriction values are on 64 bits
}restr_sz_type_e;

/*!
 * @brief This structure defines the parameter restriction table.
 */
typedef struct {
	const void *p_Table; //!< Point on the restriction table value
	uint8_t u8_restr;    //!< [0-2]: Restriction type ; [3-5] : Restriction size.
	uint8_t u8_nb;       //!< Number of element into the restriction table value
}restr_s;

#ifdef PARAM_USE_ADDRESS
#define INIT_ACCESS_TABLE(id, loc, rem, eff, var, restr_id) [id] = { \
		.u8_access = ( (loc<<LOC_ACCESS_POS) | (rem<<REM_ACCESS_POS) | (eff<<EFF_ACCESS_POS) ), \
		.u8_size = (uint8_t)sizeof(var), \
		.u32_address = (uint32_t)(&var), \
		.u8_restId = restr_id \
}
#else // PARAM_USE_OFFSET
/*!
 * @def INIT_ACCESS_TABLE
 * @brief This macro help to initialize one line into the access table
 */
#define INIT_ACCESS_TABLE(id, loc, rem, eff, ref, size, off, restr_id) [id] = { \
	.u8_access = ( (loc<<LOC_ACCESS_POS) | (rem<<REM_ACCESS_POS) | (eff<<EFF_ACCESS_POS) | (ref<<REF_ACCESS_POS)), \
	.u8_size = (uint8_t)size, \
	.u16_offset = (uint16_t)(off), \
	.u8_restId = restr_id \
}
#endif
/*!
 * @def INIT_RESTR_TABLE
 * @brief This macro help to initialize one line into the restriction table
 */
#define INIT_RESTR_TABLE(id, restr_type, restr_sz, restr_nb, restr_p_table) [id] = { \
	.u8_restr = ( (restr_type<<TYPE_RESTR_POS) | (restr_sz<<SZ_RESTR_POS) ),\
	.u8_nb = restr_nb, \
	.p_Table = restr_p_table \
}

#ifdef __cplusplus
}
#endif
#endif /* _PARAMETERS_DEF_H_ */

/*! @} */
/*! @} */
/*! @} */
