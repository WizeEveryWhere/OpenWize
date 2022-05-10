/**
  * @file sys_init.c
  * @brief This file implement (just for convenient) some "system" initialization
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
  * @par 1.0.0 : 2020/11/04 [GBI]
  * Initial version
  *
  *
  */

/*!
 *  @addtogroup sys
 *  @ingroup app
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"

#include "parameters_cfg.h"
#include "parameters.h"

#include "bsp.h"

#include "crypto.h"
#include "time_evt.h"
#include "phy_layer.h"

#include "phy_layer_private.h"

#include "wize_api.h"

/*!
 * @brief This is the context for the uart use as fake phy
 */
static fakeuart_device_t fakeuart_ctx;

/*!
 * @brief This store the phy device structure
 */
phydev_t sPhyDev;

/*!
 * @brief This function initialize the "system part"
 */
void Sys_Init(void)
{
	uint8_t u8LogLevel;
	uint8_t u8Tstmp;

	// Do not buffer stdout, so that single chars are output without any delay to the console.
	setvbuf(stdout, NULL, _IONBF, 0);
	// Do not buffer stdin, so that single chars are output without any delay to the console.
	setvbuf(stdin, NULL, _IONBF, 0);

  	/* Show the welcome message */
#ifndef HAS_NO_BANNER
  	printf("\n###########################################################\n");
  	printf("%s\n", WIZE_ALLIANCE_BANNER);
  	printf("\n###########################################################\n");
#endif

	// Init Logger
#ifdef LOGGER_USE_FWRITE
  	Logger_Setup((int32_t (*)(const char*, size_t, size_t, FILE*))fwrite, stdout);
#else
	Logger_Setup((int32_t (*)(const char*, FILE*))fputs, stdout);
#endif

	// Change logger level
	Logger_SetLevel( LOG_LV_FRM_OUT | LOG_LV_ERR | LOG_LV_WRN | LOG_LV_INF | LOG_LV_DBG, LOG_TSTAMP_HIRES | LOG_TSTAMP_TRUNC );


	assert(0 == Phy_PhyFake_setup(&sPhyDev, &fakeuart_ctx) );

	// Init storage
	Storage_Init(0);

   	// Setup Time Event
  	TimeEvt_Setup();
	// setup wize device
	WizeApi_Setup(&sPhyDev);
  	WizeApi_Enable(1);
}

/*!
 * @brief This function finalize the "system part"
 */
void Sys_Fini(void)
{
}


/*!
 * @brief Start the RTOS scheduler
 */
__attribute__ (( always_inline )) void Sys_Start(void)
{
    /* Start scheduler */
	vTaskStartScheduler();
}

#ifdef __cplusplus
}
#endif

/*! @} */
