/**
  * @file utils_secure.h
  * @brief This file expose some function that required to be in secure area.
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
  * @par 1.0.0 : 2020/12/31[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup crypto
 * @{
 *
 */
#ifndef Crypto_UTILS_SECURE_H_
#define Crypto_UTILS_SECURE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern void *secure_memcpy (void *__restrict __dest,
                                    const void *__restrict __src, size_t __n);
extern void *secure_memset (void *__s, int __c, size_t __n);

#ifdef SECURE
#define memcpy(...) secure_memcpy(__VA_ARGS__)
#define memset(...) secure_memset(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif
#endif /* Crypto_UTILS_SECURE_H_ */

/*! @} */
