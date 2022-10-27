/**
  * @file decomp.c
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

#include "decomp.h"
#include "lzma_decode.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifndef LZMA_USE_DYN_ALLOCATOR

static void *_lzma_allocate_(ISzAllocPtr p, size_t size);
static void _lzma_free_(ISzAllocPtr p, void *address);

#else

static void *_lzma_allocate_(ISzAllocPtr p, size_t size) { (void)p; return (size == 0)?(NULL):malloc(size);
static void _lzma_free_(ISzAllocPtr p, void *address) { (void)p; free(address); }

#endif

__attribute__((weak))
const ISzAlloc lzma_allocator = { &_lzma_allocate_, &_lzma_free_ };


/******************************************************************************/
__attribute__((weak))
void InitSeqInstream(CSeqInStream *stream, uint32_t base, size_t length, SRes (*read_fct)(const ISeqInStream *p, void *buf, size_t *size) )
{
	stream->vt.Read = read_fct;
	stream->pos = 0;
	stream->org = base;
	stream->size = length;
	stream->wres = SZ_OK;
}

__attribute__((weak))
void InitSeqOutstream(CSeqOutStream *stream, uint32_t base, size_t length, size_t (*write_fct)(const ISeqOutStream *p, const void *buf, size_t size) )
{
	stream->vt.Write = write_fct;
	stream->pos = 0;
	stream->org = base;
	stream->size = length;
	stream->wres = SZ_OK;
	// check base address alignment
	// check length alignment
	// erase flash ????
}

__attribute__((weak))
SRes read_fn(const ISeqInStream *pp, void *buf, size_t *size)
{
	CSeqInStream *p = CONTAINER_FROM_VTBL(pp, CSeqInStream, vt);
	uint32_t u32Sz = *size;
	uint32_t u32Address = p->org + p->pos;

	if (p->pos >= p->size)
	{
		// eof
		*size = 0;
		p->wres = SZ_ERROR_INPUT_EOF;
		return SZ_ERROR_INPUT_EOF;
	}
	// check space
	if (p->pos + u32Sz >= p->size)
	{
		// out of space, so restrict to available sapce
		u32Sz = (p->size - p->pos);
	}

#warning "read_fn have to be implemented"
	/*
	 * Add code here
	 * e.g : memcpy(buf, (void*)u32Address, u32Sz);
	 *
	 */

	*size = u32Sz;
	p->wres = SZ_OK;
	p->pos += u32Sz;
	return SZ_OK;
}

__attribute__((weak))
size_t write_fn(const ISeqOutStream *pp, const void *buf, size_t size)
{
	CSeqOutStream *p = CONTAINER_FROM_VTBL(pp, CSeqOutStream, vt);
	uint32_t u32Sz = size;
	uint32_t u32Address = p->org + p->pos;

	if (p->pos >= p->size)
	{
		// eof
		p->wres = SZ_ERROR_OUTPUT_EOF;
		return 0;
	}
	// check required space
	if (p->pos + size >= p->size)
	{
		// out of space, so restrict to available sapce
		u32Sz = (p->size - p->pos);
	}

#warning "write_fn have to be implemented"
	/*
	 * Add code here
	 * e.g :
	 *
	 * 	uint32_t u32NbDword;
	 * 	u32NbDword = u32Sz / sizeof(uint64_t);
	 *
	 * 	if ( BSP_Flash_Write(u32Address, buf, u32NbDword) != DEV_SUCCESS)
	 * 	{
	 * 		if ( memcmp((void*)u32Address, buf, u32Sz) )
	 * 		{
	 * 			// error
	 * 			p->wres = SZ_ERROR_WRITE;
	 * 			return 0;
	 * 		}
	 * 	}
	*/

	p->wres = SZ_OK;
	p->pos += u32Sz;
	return u32Sz;
}

/******************************************************************************/
#ifndef LZMA_USE_DYN_ALLOCATOR

static uint8_t _cnt_array_[DECOMP_CNT_SZ];
static uint8_t _dict_[DECOMP_DICT_SZ];
static int _n_call_ = 0;

static void *_lzma_allocate_(ISzAllocPtr p, size_t size)
{
	(void)p;

	if (_n_call_ == 0)
	{
		if (size > DECOMP_CNT_SZ)
		{
			return NULL;
		}
		_n_call_++;
		return (void*)_cnt_array_;
	}
	else if (_n_call_ == 1)
	{
		if (size > DECOMP_DICT_SZ)
		{
			return NULL;
		}
		_n_call_++;
		return (void*)_dict_;
	}
	else
	{
		return NULL;
	}
}

static void _lzma_free_(ISzAllocPtr p, void *address)
{
	(void)p;

	if ((size_t)address == 0)
	{
		return;
	}

	if (_n_call_ > 0)
	{
		_n_call_--;
	}
}

#endif

/******************************************************************************/

#ifdef __cplusplus
}
#endif
