#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(Samples_Decomp);

#include "decomp.h"
#include "lzma_decode.h"

/******************************************************************************/
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
		// out of space, so restrict to available space
		u32Sz = (p->size - p->pos);
	}

	memcpy(buf, (void*)u32Address, u32Sz);

	*size = u32Sz;
	p->wres = SZ_OK;
	p->pos += u32Sz;
	return SZ_OK;
}

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
		// out of space, so restrict to available space
		u32Sz = (p->size - p->pos);
	}

	memcpy((void*)u32Address, buf, u32Sz);

	p->wres = SZ_OK;
	p->pos += u32Sz;
	return u32Sz;
}

/******************************************************************************/
#include "_img.h"
#include "helper/_img_helper.h"
// #include "version.h"

IMPORT_BIN(".rodata", "golden/outfile.bin", InFile);
extern const uint8_t InFile[];
extern const uint8_t _sizeof_InFile[];

IMPORT_BIN(".rodata", "golden/app.bin", OrgFile);
extern const uint8_t OrgFile[];
extern const uint8_t _sizeof_OrgFile[];

size_t InFile_size = (size_t)_sizeof_InFile;
size_t OrgFile_size = (size_t)_sizeof_OrgFile;

/******************************************************************************/

#define DESTINATION_MAX_KB_SZ (200UL)
#define DESTINATION_MAX_SZ ((DESTINATION_MAX_KB_SZ) * 1024UL)

uint8_t pDest[DESTINATION_MAX_SZ];

CSeqInStream seqInStream;
CSeqOutStream seqOutStream;

/******************************************************************************/

TEST_SETUP(Samples_Decomp)
{

}

TEST_TEAR_DOWN(Samples_Decomp)
{

}

/******************************************************************************/

TEST(Samples_Decomp, test_Decomp_InStream_EOF)
{
#define IN_STREAM_SZ 10
	uint8_t inStreamBuf[IN_STREAM_SZ];
	uint8_t inBuf[15];
	SRes res;
	size_t inSize = sizeof(inBuf);

	InitSeqInstream(&seqInStream, inStreamBuf, IN_STREAM_SZ, read_fn);
	res = ISeqInStream_Read(&seqInStream.vt, inBuf, &inSize);
	TEST_ASSERT_EQUAL_INT_MESSAGE(IN_STREAM_SZ, inSize, "InStream read number error");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_OK, res, "InStream read res error");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_OK, seqInStream.wres, "InStream read wres error");

	res = ISeqInStream_Read(&seqInStream.vt, inBuf, &inSize);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, inSize, "InStream 2nd read error");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_INPUT_EOF, res, "InStream EOF res");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_INPUT_EOF, seqInStream.wres, "InStream EOF wres");
}

TEST(Samples_Decomp, test_Decomp_OutStream_EOF)
{
#define OUT_STREAM_SZ 10
	uint8_t outBuf[15];
	size_t nb_write;
	InitSeqOutstream(&seqOutStream, (uint32_t)pDest, OUT_STREAM_SZ, write_fn);

	nb_write = ISeqOutStream_Write(&seqOutStream.vt, outBuf, sizeof(outBuf));
	TEST_ASSERT_EQUAL_INT_MESSAGE(OUT_STREAM_SZ, nb_write, "OutStream write number error");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_OK, seqOutStream.wres, "OutStream write wres error");

	nb_write = ISeqOutStream_Write(&seqOutStream.vt, outBuf, sizeof(outBuf));
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, nb_write, "OutStream 2nd write error");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_OUTPUT_EOF, seqOutStream.wres, "OutStream EOF wres");
}

/******************************************************************************/
#define TEST_LZMA_PROPS_SZ 5
#define TEST_LZMA_UNCOMPRESSED_SZ 8
#define TEST_LZMA_PB 2
#define TEST_LZMA_LP 0
#define TEST_LZMA_LC 3
/* (from  lzma doc)
header: 5 bytes of LZMA properties and 8 bytes of uncompressed size

LZMA compressed file format
---------------------------
Offset Size Description
  0     1   Special LZMA properties (lc,lp, pb in encoded form)
  1     4   Dictionary size (little endian)
  5     8   Uncompressed size (little endian). -1 means unknown size
 13         Compressed data

LZMA properties:
name  Range          Description

  lc  [0, 8]         the number of "literal context" bits
  lp  [0, 4]         the number of "literal pos" bits
  pb  [0, 4]         the number of "pos" bits

dictSize  [0, 2^32 - 1]  the dictionary size

Then :
state_size = (4 + (1.5 << (lc + lp))) KB
With :
| lc | lp | state_size KB
| 3  | 0  | 16
| 2  | 0  | 10
| 3  | 1  | 28
*/
TEST(Samples_Decomp, test_Decomp_Allocator_Fail)
{
	// TODO :
	printf(" *** TODO *** ");
}

TEST(Samples_Decomp, test_Decomp_AllocateDictSize_Fail)
{
#define IN_STREAM_SZ (TEST_LZMA_PROPS_SZ + TEST_LZMA_UNCOMPRESSED_SZ + 20)
	SRes res;
	uint8_t header[IN_STREAM_SZ];
	uint8_t pb, lp, lc;
	pb = TEST_LZMA_PB;
	lp = TEST_LZMA_LP;
	lc = TEST_LZMA_LC;
	header[0] = (Byte)((pb * 5 + lp) * 9 + lc);
	// dict
	*(uint32_t*)(&(header[1])) = 32768;

	InitSeqInstream(&seqInStream, header, IN_STREAM_SZ, read_fn);
	InitSeqOutstream(&seqOutStream, (uint32_t)pDest, DESTINATION_MAX_SZ, write_fn);
	res = Decode(&seqOutStream, &seqInStream.vt);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_MEM, res, "Dict");
}

TEST(Samples_Decomp, test_Decomp_AllocateCntSize_Fail)
{
#define IN_STREAM_SZ (TEST_LZMA_PROPS_SZ + TEST_LZMA_UNCOMPRESSED_SZ + 20)
	SRes res;
	uint8_t header[IN_STREAM_SZ];
	uint8_t pb, lp, lc;
	pb = TEST_LZMA_PB;
	lp = 1;
	lc = TEST_LZMA_LC;
	// cnt
	header[0] = (Byte)((pb * 5 + lp) * 9 + lc);

	InitSeqInstream(&seqInStream, header, IN_STREAM_SZ, read_fn);
	InitSeqOutstream(&seqOutStream, (uint32_t)pDest, DESTINATION_MAX_SZ, write_fn);
	res = Decode(&seqOutStream, &seqInStream.vt);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_MEM, res, "Cnt");
}

TEST(Samples_Decomp, test_Decomp_TooLargeFile_Fail)
{
	// TODO :
	printf(" *** TODO *** ");
}
/******************************************************************************/

TEST(Samples_Decomp, test_Decomp_Decode_Fail)
{
	SRes res;
#define IN_STREAM_SZ 20
#define OUT_STREAM_SZ 20
	uint8_t inStreamBuf[IN_STREAM_SZ];
	uint8_t outStreamBuf[OUT_STREAM_SZ];

	InitSeqInstream(&seqInStream, (uint32_t)inStreamBuf, IN_STREAM_SZ, read_fn);
	InitSeqOutstream(&seqOutStream, (uint32_t)outStreamBuf, OUT_STREAM_SZ, write_fn);

	res = Decode(&seqOutStream.vt, NULL);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_READ, res, "InStream Read fn");

	res = Decode(NULL, &seqInStream.vt);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_WRITE, res, "OutStream Write fn");

	// TODO :
	//TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_ERROR_DATA, res, "Decode state");
}

/******************************************************************************/

TEST(Samples_Decomp, test_Decomp_Small_Success)
{
	SRes res;

	img_header_t *pHeader = (img_header_t *)InFile;
	printf("\n");
	printf("*** Info\n");
	printf("*** \tInFile size : %i\n", InFile_size);
	printf("*** \tInFile address : %x\n", InFile);

	printf("*** Header\n");
	printf("*** \tmagic : %x\n", pHeader->magic);
	printf("*** \tsize : %i\n", pHeader->hdr_sz);
	printf("*** \ttype : %i\n", pHeader->type);

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(MAGIC_WORD_IMG, pHeader->magic, "Header magic number error");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(sizeof(img_header_t), pHeader->hdr_sz, "Header size error");

	uint32_t img_start = (uint32_t)InFile + pHeader->hdr_sz;
	size_t img_sz = InFile_size - pHeader->hdr_sz;

	InitSeqInstream(&seqInStream, img_start, img_sz, read_fn);
	InitSeqOutstream(&seqOutStream, (uint32_t)pDest, DESTINATION_MAX_SZ, write_fn);

	res = Decode(&seqOutStream.vt, &seqInStream.vt);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SZ_OK, res, "Decode return");
	TEST_ASSERT_EQUAL_MEMORY_MESSAGE(pDest, OrgFile, OrgFile_size, "The computed buffer does not match the expected one");
}

/******************************************************************************/
