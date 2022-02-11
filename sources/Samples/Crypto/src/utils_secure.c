/**
  * @file: utils_secure.c
  * @brief: This file expose some function that required to be in secure area.
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
  * 1.0.0 : 2020/12/31[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup Crypto
 * @{
 */

#include <stddef.h>

/*!
  * @fn void *secure_memcpy (void *__restrict __dest,
  *                                    const void *__restrict __src, size_t __n)
  *
  * @brief This function is intended to replaced memcpy into secure area.
  *
  * @param [in] __dest The destination pointer
  * @param [in] __src  The source pointer
  * @retval return the destination pointer
  *
  */
void *secure_memcpy (void *__restrict __dest, const void *__restrict __src,
		size_t __n){
    char* dst8 = (char*)__dest;
    const char* src8 = (char*)__src;
    --src8;
    --dst8;

    while (__n--) {
        *++dst8 = *++src8;
    }
    return __dest;
}

/*!
  * @fn void *secure_memset (void *__s, int __c, size_t __n)
  *
  * @brief This function is intended to replaced memset into secure area.
  *
  * @param [in] __s The destination pointer
  * @param [in] __c The character value to set
  * @param [in] __n The number of char to set
  * @retval return the destination pointer
  *
  */
void *secure_memset (void *__s, int __c, size_t __n){
	  unsigned char *ptr = __s;
	  while (__n-- > 0)
	    *ptr++ = __c;
	  return __s;
}

/*! @} */
/*! @} */
/*! @} */
