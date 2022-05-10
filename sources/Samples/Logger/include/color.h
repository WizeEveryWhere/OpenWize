/**
  * @file color.h
  * @brief This file contains coloring useful macro
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
  * @par 1.0.0 : 2020/11/19[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup logger
 * @{
 *
 */
#ifndef _COLOR_H_
#define _COLOR_H_
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @cond INTERNAL
 * @{
 */

#define STR(s) #s
#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7
#define FG 3
#define BG 4
#define COLOR(type, color) STR( type )STR( color )

#define ESC_CHAR "\33["
#define END_ESC ESC_CHAR"0m"
// Format
#define BOLD STR(1)
#define FGC(color) COLOR(FG, color)
#define BGC(color) COLOR(BG, color)
#define SGR(...) ESC_CHAR __VA_ARGS__ "m"

/*!
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif
#endif /* _COLOR_H_ */

/*! @} */
