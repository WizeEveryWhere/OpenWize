/**
  * @file: bsp_flash.h
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
  * 1.0.0 : 2021/02/19[GBI]
  * Initial version
  *
  *
  */
#ifndef _BSP_FLASH_H_
#define _BSP_FLASH_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

dev_res_e BSP_Flash_Erase(uint32_t u32Page);
dev_res_e BSP_Flash_EraseArea(uint32_t u32Address, uint32_t u32NbBytes);
dev_res_e BSP_Flash_Write(uint32_t u32Address, uint64_t *pData, uint32_t u32NbDword);
uint32_t BSP_Flash_Store(uint32_t u32DestAddr, void* pData, uint32_t u32NbBytes);
uint32_t BSP_Flash_GetPage(uint32_t u32Addr);

#ifdef __cplusplus
}
#endif
#endif /* _BSP_FLASH_H_ */
