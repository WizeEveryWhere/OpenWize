/**
  * @file net_api.h
  * @brief // TODO This file ...
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
  * 1.0.0 : 2020/09/10[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Device
 * @{
 * @ingroup Wize
 * @{
 *
 */

#ifndef _NET_API_H_
#define _NET_API_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "proto.h"

/*!
 * @brief This structure is used to hold the device information.
 */
typedef union
{
	uint8_t  aDevInfo[8]; //!< Device info .
	struct
	{
		uint8_t  aManuf[2]; //!< Device manufacturer (8 BCD, LSB first).
		union
		{
			uint8_t  aAddr[6];  //!< Unique device identification number (8 BCD, LSB first).
			struct
			{
				uint8_t  aNum[4];   //!< Unique device identification number (8 BCD, LSB first).
				uint8_t  u8Ver;     //!< Device version (BCD).
				uint8_t  u8Type;    //!< Device type (BCD)
			};
		};
	};
} device_id_t;

#ifdef __cplusplus
}
#endif
#endif /* _NET_API_H_ */

/*! @} */
/*! @} */
/*! @} */
