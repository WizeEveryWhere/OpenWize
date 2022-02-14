/**
  * @file parameters.c
  * @brief This file expose all parameter function's for local, remote and
  *         direct access.
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
  * 1.0.0 : 2019/11/20 00:08:20 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Samples
 * @{
 *
 */
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "parameters_def.h"

/*!
 * @brief This external variable hold the parameters table value size
 */
extern const uint16_t u16_ParamValueSz;
/*!
 * @brief This external variable hold the parameters table value
 */
extern uint8_t a_ParamValue[];

/*!
 * @brief This external variable hold the parameters access table size
 */
extern const uint8_t u8_ParamAccessCfgSz;
/*!
 * @brief This external variable hold parameters access table
 */
extern const param_s a_ParamAccess[];

/*!
 * @brief This external variable hold the parameters restriction table size
 */
extern const uint8_t u8_ParamRestrCfgSz;
/*!
 * @brief This external variable hold the parameters restriction table
 */
extern const restr_s a_ParamRestr[];

static uint8_t _param_access_(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir, uint8_t u8_Access);
static restr_s* _get_restr_add_(uint8_t u8_Id);
static uint8_t _check_conform_modulo_(restr_s *p_Restr, uint8_t* p_Data);
static uint8_t _check_conform_enum_(restr_s *p_Restr, uint8_t* p_Data);
static uint8_t _check_conform_range_(restr_s *p_Restr, uint8_t* p_Data);

/*!
  * @static
  * @brief  This private function deal with access to parameter table
  * 
  * @param [in]     u8_Id     The parameter Id.
  * @param [in,out] p_Data  Pointer on source or destination data.
  * @param [in]     u8_Dir    The access direction (0:read; 1:write).
  * @param [in]     u8_Access The access rights.
  * @retval  0: Error
  * @retval  1: Success
  * 
  */
static uint8_t _param_access_(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir, uint8_t u8_Access){
    uint8_t u8_ret, u8_isAuth;
    void *pvSrc, *pvDest;
    u8_isAuth = (u8_Dir)?(u8_Access & WO):(u8_Access & RO);
    // if u8_Dir try to write and !(u8_Access & WO) not writable, u8_isAuth = 0
    // if !u8_Dir try to read and !(u8_Access & RO) not readable, u8_isAuth = 0
    if (u8_isAuth){ // authorized to access
        if (u8_Dir) {
                pvSrc = (void*)p_Data;
#ifdef PARAM_USE_ADDRESS
                pvDest = (void*)(a_ParamAccess[u8_Id].u32_address);
#else
                pvDest = (void*)(&(a_ParamValue[a_ParamAccess[u8_Id].u16_offset]));
#endif
        }
        else {
                pvDest = (void*)p_Data;
#ifdef PARAM_USE_ADDRESS
                pvSrc = (void*)(a_ParamAccess[u8_Id].u32_address);
#else
                pvSrc = (void*)(&(a_ParamValue[a_ParamAccess[u8_Id].u16_offset]));
#endif                
        }
        if (pvDest == NULL || pvSrc == NULL) {
            u8_ret = 0;
        }
        else {
        	//_param_lock_((void*)&(a_ParamAccess[u8_Id].u8_access));
            memcpy(pvDest, pvSrc, a_ParamAccess[u8_Id].u8_size);
            //_param_unlock_((void*)&(a_ParamAccess[u8_Id].u8_access));
            u8_ret = 1;
        }
    }
    else {
        u8_ret = 0;
    }
    return u8_ret;
}

/*!
  * @static
  * @brief Get the restriction table address of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return  The restriction table address.
  */
static restr_s* _get_restr_add_(uint8_t u8_Id){
	if ( (a_ParamAccess[u8_Id].u8_restId > 0) &&
		 (a_ParamAccess[u8_Id].u8_restId < u8_ParamRestrCfgSz )
		){
		return (restr_s*)(&(a_ParamRestr[a_ParamAccess[u8_Id].u8_restId]));
	}
	return (NULL);
}

/*!
  * @static
  * @brief Check that given value conform to one of the enum value.
  *
  * @param [in] *p_Restr point on the restriction table
  * @param [in] *p_Data point on the data to check
  * @retval 0 : not conform
  * @retval 1 : conform
  */
static uint8_t _check_conform_enum_(restr_s *p_Restr, uint8_t* p_Data)
{
	uint8_t i;
	uint8_t u8_ret = 0;
	restr_sz_type_e e_size;
	if (p_Restr != NULL && p_Restr->p_Table != NULL && p_Data != NULL){
		e_size = GET_RESTR_SZ(p_Restr->u8_restr);
		for (i=0; i < p_Restr->u8_nb; i++){
			if ( e_size == RESTR_8BITS){
				if ( (u8_ret = CHECK_ENUM_VAL(((uint8_t*)p_Data)[0], ((uint8_t*)p_Restr->p_Table)[i])) ){
					// we found the value
					break;
				}
			}
			else if (e_size == RESTR_16BITS) {
				if ( (u8_ret = CHECK_ENUM_VAL(((uint16_t*)p_Data)[0], ((uint16_t*)p_Restr->p_Table)[i])) ){
					// we found the value
					break;
				}
			}
			else if (e_size == RESTR_32BITS) {
				if ( (u8_ret = CHECK_ENUM_VAL(((uint32_t*)p_Data)[0], ((uint32_t*)p_Restr->p_Table)[i])) ){
					// we found the value
					break;
				}
			}
			else if (e_size == RESTR_64BITS) {
				if ( (u8_ret = CHECK_ENUM_VAL(((uint64_t*)p_Data)[0], ((uint64_t*)p_Restr->p_Table)[i])) ){
					// we found the value
					break;
				}
			}
			else {
				// e_size seems to be unknown, so stop the loop
				break;
			}
		}
	}
	return u8_ret;
}

/*!
  * @static
  * @brief Check that given value conform to the modulo value (see @link CHECK_MODULO @endlink).
  *
  * @param [in] *p_Restr point on the restriction table
  * @param [in] *p_Data point on the data to check
  * @retval 0 : not conform
  * @retval 1 : conform
  */
static uint8_t _check_conform_modulo_(restr_s *p_Restr, uint8_t* p_Data)
{
	uint8_t u8_ret = 0;
	restr_sz_type_e e_size;
	if (p_Restr != NULL && p_Restr->p_Table != NULL && p_Data != NULL){
		e_size = GET_RESTR_SZ(p_Restr->u8_restr);

		if ( e_size == RESTR_8BITS){
			u8_ret = CHECK_MODULO( ((uint8_t*)p_Data)[0], ((uint8_t*)p_Restr->p_Table)[0] );
		}
		else if (e_size == RESTR_16BITS) {
			u8_ret = CHECK_MODULO( ((uint16_t*)p_Data)[0], ((uint16_t*)p_Restr->p_Table)[0] );
		}
		else if (e_size == RESTR_32BITS) {
			u8_ret = CHECK_MODULO( ((uint32_t*)p_Data)[0], ((uint32_t*)p_Restr->p_Table)[0] );
		}
		else if (e_size == RESTR_64BITS) {
			u8_ret = CHECK_MODULO( ((uint64_t*)p_Data)[0], ((uint64_t*)p_Restr->p_Table)[0] );
		}
		// else {} // e_size seems to be unknown
	}
	return u8_ret;
}

/*!
  * @static
  * @brief Check that given value conform to the range value (see @link CHECK_RANGE @endlink).
  *
  * @param [in] *p_Restr point on the restriction table
  * @param [in] *p_Data point on the data to check
  * @retval 0 : not conform
  * @retval 1 : conform
  */
static uint8_t _check_conform_range_(restr_s *p_Restr, uint8_t* p_Data)
{
	uint8_t u8_ret = 0;
	restr_sz_type_e e_size;
	if (p_Restr != NULL && p_Restr->p_Table != NULL && p_Data != NULL){
		e_size = GET_RESTR_SZ(p_Restr->u8_restr);

		if ( e_size == RESTR_8BITS){
			u8_ret = CHECK_RANGE( ((uint8_t*)p_Data)[0], ((uint8_t*)p_Restr->p_Table)[0], ((uint8_t*)p_Restr->p_Table)[1] );
		}
		else if (e_size == RESTR_16BITS) {
			u8_ret = CHECK_RANGE( ((uint16_t*)p_Data)[0], ((uint16_t*)p_Restr->p_Table)[0], ((uint16_t*)p_Restr->p_Table)[1] );
		}
		else if (e_size == RESTR_32BITS) {
			u8_ret = CHECK_RANGE( ((uint32_t*)p_Data)[0], ((uint32_t*)p_Restr->p_Table)[0], ((uint32_t*)p_Restr->p_Table)[1] );
		}
		else if (e_size == RESTR_64BITS) {
			u8_ret = CHECK_RANGE( ((uint64_t*)p_Data)[0], ((uint64_t*)p_Restr->p_Table)[0], ((uint64_t*)p_Restr->p_Table)[1] );
		}
		// else {} // e_size seems to be unknown
	}
	return u8_ret;
}


/*!
  * @fn  void Param_Init(const uint8_t *p_Param)
  * @brief Init parameters by with default values
  *
  * @param [in] p_Param Pointer on input parameters table.
  * @return None
  */
void Param_Init(const uint8_t *p_Param)
{
	memcpy(a_ParamValue, p_Param, u16_ParamValueSz);
}

/*!
  * @fn  inline uint8_t Param_IsValidId(uint8_t u8_Id)
  * @brief Check if Id is in acess table.
  *
  * @param [in] u8_Id The parameter Id.
  * @retval 0 : not valid
  * @retval 1 : valid
  */
uint8_t Param_IsValidId(uint8_t u8_Id){
    return ((u8_Id >= u8_ParamAccessCfgSz)?(0):(1) );
}

/*!
  * @fn  inline uint8_t Param_GetSize(uint8_t u8_Id)
  * @brief Get the parameter size.
  *
  * @param [in] u8_Id The parameter Id.
  * @return     The parameter size.
  */
uint8_t Param_GetSize(uint8_t u8_Id){
    return a_ParamAccess[u8_Id].u8_size;
}

/*!
  * @fn inline param_access_e Param_GetLocAccess(uint8_t u8_Id)
  * @brief Get the local access rights field of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return     The local access rights field.
  */
param_access_e Param_GetLocAccess(uint8_t u8_Id){
    return (param_access_e)( (a_ParamAccess[u8_Id].u8_access >> LOC_ACCESS_POS) & LOC_ACCESS_MSK);
}

/*!
  * @fn  inline param_access_e Param_GetRemAccess(uint8_t u8_Id)
  * @brief Get the remote access rights field of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return     The remote access rights field.
  */
param_access_e Param_GetRemAccess(uint8_t u8_Id){
    return (param_access_e)( (a_ParamAccess[u8_Id].u8_access >> REM_ACCESS_POS) & REM_ACCESS_MSK);
}

/*!
  * @fn inline param_effective_e Param_GetEffect(uint8_t u8_Id)
  * @brief Get the "effective" field of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return     The "effective" field.
  */
param_effective_e Param_GetEffect(uint8_t u8_Id){
    return (param_effective_e)( (a_ParamAccess[u8_Id].u8_access >> EFF_ACCESS_POS) & EFF_ACCESS_MSK);
}

/*!
  * @fn inline param_effective_e Param_GetReferenced(uint8_t u8_Id)
  * @brief Get the "referenced" field of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return     The "referenced" field.
  */
param_ref_e Param_GetReferenced(uint8_t u8_Id){
    return (param_ref_e)( (a_ParamAccess[u8_Id].u8_access >> REF_ACCESS_POS) & REF_ACCESS_MSK);
}


/*!
  * @fn  inline uint32_t Param_GetAddOf(uint8_t u8_Id)
  * @brief   Get the memory address of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return     The parameter address
  */
uint32_t Param_GetAddOf(uint8_t u8_Id){
#ifdef PARAM_USE_ADDRESS
    return a_ParamAccess[u8_Id].u32_address;
#else
    return (uint32_t)(&(a_ParamValue[a_ParamAccess[u8_Id].u16_offset]));
#endif
}

/*!
  * @fn  inline uint8_t Param_GetRestrId(uint8_t u8_Id)
  * @brief Get the restriction id of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return  The restriction id field.
  */
uint8_t Param_GetRestrId(uint8_t u8_Id){
    return  (a_ParamAccess[u8_Id].u8_restId );
}

/*!
  * @fn const void* Param_GetRestrTableAdd(uint8_t u8_Id)
  * @brief Get the restriction table address of the parameter
  *
  * @param [in] u8_Id The parameter Id.
  * @return  The restriction table address.
  */
const void* Param_GetRestrTableAdd(uint8_t u8_Id){
	if ( (a_ParamAccess[u8_Id].u8_restId > 0) &&
		 (a_ParamAccess[u8_Id].u8_restId < u8_ParamRestrCfgSz )
		){
		return ( (a_ParamRestr[a_ParamAccess[u8_Id].u8_restId].p_Table) );
	}
	return (NULL);
}

/*!
  * @fn uint8_t Param_CheckConformity(uint8_t u8_Id, uint8_t* p_Data)
  * @brief Check that given value conform to its restriction definition.
  *
  * @param [in] u8_Id id of the parameter to check
  * @param [in] *p_Data point on the data to check
  * @retval 0 : not conform
  * @retval 1 : conform
  */
uint8_t Param_CheckConformity(uint8_t u8_Id, uint8_t* p_Data){
	uint8_t u8_ret = 1;
	restr_type_e e_type;
	restr_s *p_restr = _get_restr_add_(u8_Id);
	if (  p_restr != NULL){
		e_type = GET_RESTR_TYPE(p_restr->u8_restr);
		switch ( e_type ) {
			case RESTR_MODULO:
				u8_ret = _check_conform_modulo_(p_restr, p_Data);
				break;
			case RESTR_RANGE:
				u8_ret = _check_conform_range_(p_restr, p_Data);
				break;
			case RESTR_ENUM:
				u8_ret = _check_conform_enum_(p_restr, p_Data);
				break;
			default :
				break;
		}
	}
	return u8_ret;
}

/*!
  * @fn  uint8_t Param_Access(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir)
  * @brief  Pseudo-Direct access to the parameters table
  * 
  * @param [in]     u8_Id    The parameter Id.
  * @param [in,out] p_Data Pointer on source or destination data.
  * @param [in]     u8_Dir   The access direction (0:read; 1:write).
  * @retval  0: Error
  * @retval  1: success
  */
uint8_t Param_Access(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir){
    uint8_t u8_ret;
    u8_ret = _param_access_(u8_Id, p_Data, u8_Dir, 0b11);
#ifdef USE_CONST_PARAM_ERR_MSG
    if (u8_ret == 0) {
        if (u8_Dir) { //we try to write
            printf("write access error\n");
        }
        else { // we try to read
            printf("read access error\n");
        }
    }
#endif
    return u8_ret;
}

/*!
  * @fn  uint8_t Param_LocalAccess(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir)
  * @brief  Local access to the parameters table
  * 
  * @param [in]     u8_Id    The parameter Id.
  * @param [in,out] p_Data Pointer on source or destination data.
  * @param [in]     u8_Dir   The access direction (0:read; 1:write).
  * @retval  0: access is forbidden
  * @retval  1: access granted
  * 
  */
uint8_t Param_LocalAccess(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir){
    uint8_t u8_ret, u8_access;
    
    u8_access = Param_GetLocAccess(u8_Id);
    u8_ret = _param_access_(u8_Id, p_Data, u8_Dir, u8_access);
#ifdef USE_CONST_PARAM_ERR_MSG
    if (u8_ret == 0) {
        if (u8_Dir) { //we try to write
            printf("local write access is forbidden\n");
        }
        else { // we try to read
            printf("local read access is forbidden\n");
        }
    }
#endif
    return u8_ret;
}

/*!
  * @fn  uint8_t Param_RemoteAccess(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir)
  * @brief  Remote access to the parameters table
  * 
  * @param [in]     u8_Id    The parameter Id.
  * @param [in,out] p_Data Pointer on source or destination data.
  * @param [in]     u8_Dir   The access direction (0:read; 1:write).
  * @retval  0: access is forbidden; 1: access granted
  * 
  */
uint8_t Param_RemoteAccess(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir){
    uint8_t u8_ret, u8_access;
    
    u8_access = Param_GetRemAccess(u8_Id);
    u8_ret = _param_access_(u8_Id, p_Data, u8_Dir, u8_access);

#ifdef USE_CONST_PARAM_ERR_MSG
    if (u8_ret == 0) {
        if (u8_Dir) { //we try to write
            printf("remote write access is forbidden\n");
        }
        else { // we try to read
            printf("remote read access is forbidden\n");
        }
    }
#endif
    return u8_ret;
}

/*! @} */
