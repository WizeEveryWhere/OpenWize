/**
  * @file lzma_decode.c
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
  * @par 1.0.0 : 2022/08/16 [TODO: your name]
  * Initial version
  *
  */

#include "lzma_decode.h"
#include "LzmaDec.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
extern ISzAlloc lzma_allocator;

/******************************************************************************/
#ifndef IN_BUF_SIZE
	#define IN_BUF_SIZE  (512UL)
#endif

#ifndef OUT_BUF_SIZE
	#define OUT_BUF_SIZE (768UL)
#endif

static SRes _decode_(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream, uint32_t unpackSize);

/******************************************************************************/

SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream)
{
	int32_t ret;
	uint32_t unpackSize;
	int i;
	SRes res = 0;

	CLzmaDec state;

	if( (lzma_allocator.Alloc == NULL) || (lzma_allocator.Free == NULL) )
	{
		return SZ_ERROR_MEM;
	}
	if ( !inStream || !(inStream->Read))
	{
		return SZ_ERROR_READ;
	}
	if ( !outStream || !(outStream->Write))
	{
		return SZ_ERROR_WRITE;
	}

	/* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
	uint8_t header[LZMA_PROPS_SIZE + 8];
	size_t header_sz = sizeof(header);
	/* Read and parse header */
	ret = ISeqInStream_Read(inStream, header, &header_sz);
	//ret = SeqInStream_Read(inStream, header, sizeof(header));
	if (ret != 0) { return ret; }

	unpackSize = 0;
	for (i = 0; i < 8; i++)
	{
		unpackSize += (uint32_t)header[LZMA_PROPS_SIZE + i] << (i * 8);
	}
	LzmaDec_Construct(&state);

	ret = LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &lzma_allocator);
	if (ret != 0) { return ret; }

	res = _decode_(&state, outStream, inStream, unpackSize);

	LzmaDec_Free(&state, &lzma_allocator);
	return res;
}

/******************************************************************************/

static SRes _decode_(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
	uint32_t unpackSize)
{
	int32_t thereIsSize = (unpackSize != (uint32_t)(int32_t)-1);
	uint8_t inBuf[IN_BUF_SIZE];
	uint8_t outBuf[OUT_BUF_SIZE];
	size_t inPos = 0, inSize = 0, outPos = 0;

	ELzmaFinishMode finishMode;
	ELzmaStatus status;

	if ( !state)
	{
		return SZ_ERROR_PARAM;
	}

	LzmaDec_Init(state);
	for (;;)
	{
		SRes res;
		if (inPos == inSize)
		{
			inSize = IN_BUF_SIZE;
			res = ISeqInStream_Read(inStream, inBuf, &inSize);
			if (res != SZ_OK) { return res; }
			inPos = 0;
		}
		{
			size_t inProcessed = inSize - inPos;
			size_t outProcessed = OUT_BUF_SIZE - outPos;

			finishMode = LZMA_FINISH_ANY;
			if (thereIsSize && outProcessed > unpackSize)
			{
				outProcessed = (size_t)unpackSize;
				finishMode = LZMA_FINISH_END;
			}

			res = LzmaDec_DecodeToBuf(state,
					outBuf + outPos,
					&outProcessed,
					inBuf + inPos,
					&inProcessed,
					finishMode,
					&status);

			inPos += inProcessed;

			// Total available decompressed data from previous iteration
			size_t available_nb = outProcessed + outPos;
			// Number of aligned bytes
			size_t aligned_nb = available_nb & ~3UL;
			size_t nb_write = ISeqOutStream_Write(outStream, outBuf, aligned_nb);
			if (!nb_write) { return SZ_ERROR_WRITE; }

			// Number of unaligned bytes
			size_t unaligned_nb = available_nb - aligned_nb;
			// Move the unaligned bytes at the beginning
			if (unaligned_nb > 0)
			{
				memmove(&outBuf[0], &outBuf[aligned_nb], unaligned_nb);
			}
			outPos = unaligned_nb;
			// outPos = 0;

			unpackSize -= outProcessed;

			if (res != SZ_OK || (thereIsSize && unpackSize == 0))
			{
				return res;
			}

			if (inProcessed == 0 && outProcessed == 0)
			{
				if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
				{
					return SZ_ERROR_DATA;
				}
				return res;
			}
		}
	}
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif
