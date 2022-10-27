/**
  * @file decomp.h
  * @brief // TODO This file ...
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
  * @par 1.0.0 : 2022/10/06 [TODO: your name]
  * Initial version
  *
  */
#ifndef DECOMP_H_
#define DECOMP_H_

#include "7zTypes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LZMA_USE_DYN_ALLOCATOR
	#ifndef LZMA_CNT_KB_SZ
		/**
		 * @brief The maximum size of probability model counters.
		 * @details LZMA Decoder uses dictionary buffer and internal state structure.
		 * Internal state structure consumes :
		 *   state_size = (4 + (1.5 << (lc + lp))) KB
		 * by default (lc=3, lp=0), state_size = 16 KB.
		*/
		#define LZMA_CNT_KB_SZ        (16UL)
	#endif

	#ifndef LZMA_DICT_KB_SZ
		/** @brief The maximum size of the dictionary. */
		// LZMA payloads with a dictionary that's too large  can't be decompressed.
		#define LZMA_DICT_KB_SZ           (8UL)
	#endif

	#define DECOMP_CNT_SZ  ((LZMA_CNT_KB_SZ) * 1024UL)
	#define DECOMP_DICT_SZ ((LZMA_DICT_KB_SZ) * 1024UL)
#endif

typedef struct
{
	ISeqInStream vt;
	size_t org;
	size_t size;
	size_t pos;
	WRes wres;
	// void *lock;
} CSeqInStream;

typedef struct
{
	ISeqOutStream vt;
	size_t org;  // base address, aligned on double-woed
	size_t size; // area size in bytes
	size_t pos;  // position in the "stream"
	WRes wres;
	// void *lock;
} CSeqOutStream;

void InitSeqInstream(CSeqInStream *stream, uint32_t base, size_t length, SRes (*read_fct)(const ISeqInStream *p, void *buf, size_t *size) );
void InitSeqOutstream(CSeqOutStream *stream, uint32_t base, size_t length, size_t (*write_fct)(const ISeqOutStream *p, const void *buf, size_t size) );
SRes read_fn(const ISeqInStream *pp, void *buf, size_t *size);
size_t write_to_flash_fn(const ISeqOutStream *pp, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* DECOMP_H_ */
