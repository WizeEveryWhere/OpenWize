
/**
  * @file: sys_init.c
  * @brief: // TODO This file ...
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
  * 1.0.0 : 2020/11/04[TODO: your name]
  * Initial version
  *
  *
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

#include "storage.h"

static fakeuart_device_t fakeuart_ctx;
phydev_t sPhyDev;

extern boot_state_t gBootState;

void Sys_Init(void)
{
	uint8_t u8LogLevel;
	uint8_t u8Tstmp;
	// check wake-up(s)
	if (gBootState.state & (WKUP_PIN_MSK | INTERNAL_WKUP_MSK))
	{
		uint32_t *pSRAM2 = (uint32_t*)(0x10000000);
		if (*pSRAM2 == 0x00000000) // backup domain has been reseted
		{
			gBootState.state |= BACKUP_RESET_MSK;
			// Restore minimal context (exit from shutdown or backup domain reset)
			// - L6Cpt DATA
			// - L6Cpt RSP
			// - PeriodicInstallCpt
			// - FullPowerCpt
		}

		// check wake-up internal : RTC ALARM or RTC WAKEUP TIMER
		if (gBootState.state & INTERNAL_WKUP_MSK)
		{
		}
		// check wake-up external : WAKUPE PINs
		if (gBootState.state & WKUP_PIN_MSK)
		{
		}
	}


	// Do not buffer stdout, so that single chars are output without any delay to the console.
	setvbuf(stdout, NULL, _IONBF, 0);
	// Do not buffer stdin, so that single chars are output without any delay to the console.
	setvbuf(stdin, NULL, _IONBF, 0);
	// setup timezone
	//tzset();

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
	if ( Param_Access(LOGGER_LEVEL, (uint8_t*)(&u8LogLevel), 0) == 0 )
	{
		u8LogLevel = LOG_LV_FRM_OUT | LOG_LV_ERR | LOG_LV_WRN | LOG_LV_INF | LOG_LV_DBG;
	}
	if ( Param_Access(LOGGER_TIME_OPT, (uint8_t*)(&u8Tstmp), 0) == 0 )
	{
		u8Tstmp = LOG_TSTAMP_HIRES | LOG_TSTAMP_TRUNC;
	}
	//Logger_SetLevel( u8LogLevel, u8Tstmp );
	Logger_SetLevel( LOG_LV_FRM_OUT | LOG_LV_ERR | LOG_LV_WRN | LOG_LV_INF | LOG_LV_DBG, LOG_TSTAMP_HIRES | LOG_TSTAMP_TRUNC );

	LOG_INF("SYS boot 0x%x\n", gBootState);

	assert(0 == Phy_PhyFake_setup(&sPhyDev, &fakeuart_ctx) );

	if (gBootState.state & BACKUP_RESET_MSK)
	{
		LOG_INF("SYS Clear Ctx\n");
		WizeApi_CtxClear();
	}


	// Init storage
	Storage_Init(0);

  	// Init Time Mgr
	WizeApi_CtxRestore();
   	// Setup Time Event
  	TimeEvt_Setup();
	// setup wize device
	WizeApi_Setup(&sPhyDev);

	// Setup phy device
	//Phy_device_setup( &sPhyDev, ...);

	// Init wize device
	//WizeApi_Init(&sPhyDev);


  	WizeApi_Enable(1);
}

void Sys_Fini(void)
{
	WizeApi_CtxSave();
	// BSP_LowPower_Enter(mode);
	// BSP_Boot_Reboot(bReset);
}


//__attribute__ (( noreturn, always_inline )) inline void Sys_Start(void)
__attribute__ (( always_inline )) void Sys_Start(void)
{
    /* Start scheduler */
	vTaskStartScheduler();
}

#ifdef __cplusplus
}
#endif
