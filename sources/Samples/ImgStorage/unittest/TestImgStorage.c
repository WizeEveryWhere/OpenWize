#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(Samples_ImgStorage);

#include "img_storage_private.h"
#include "mock_crypto.h"
/******************************************************************************/
extern struct img_mgr_ctx_s sImgMgrCtx;

#include "loremIpsum.h"

uint8_t aBuff[sizeof(aLoremIpsum)] __attribute__ ((aligned (8)));
uint8_t aTargetArea[IMG_MAX_SZ] __attribute__ ((aligned (8)));

static uint8_t _local_write(uint32_t u32Addr, uint64_t *pData, uint32_t u32NbDoubleWord)
{
	uint64_t *p;
	uint32_t i;
	p = (uint64_t*)u32Addr;
	for ( i = 0; i < u32NbDoubleWord; i++)
	{
		p[i] = pData[i];
	}
	return 0;
}

static uint8_t _local_erase(uint32_t u32Addr, uint32_t u32Size)
{
	uint8_t *p;
	uint32_t i;
	p = (uint8_t*)u32Addr;
	for ( i = 0; i < u32Size; i++)
	{
		*p = 0xFF;
		p++;
	}
	return 0;
}

static uint8_t _fake_erase(uint32_t u32Addr, uint32_t u32Size)
{
	return 1;
}

static void _fill_FF_bitmap(void)
{
	uint16_t i;
	for (i = 0; i < BITMAP_BLOCK_NB; i++)
	{
		sImgMgrCtx.aBitMap[i] = 0xFF;
	}
}

static void _fill_NON_EMPTY_pend(void)
{
	uint16_t i;
	for (i = 0; i < PENDING_LINE_NB; i++)
	{
		sImgMgrCtx.sPendCtrl[i].EMPTY = 1;
	}
}


uint32_t u32ExpectedSz;
uint8_t *pExpectedPtr;
uint8_t *pExpectedSHA256Ptr;

uint8_t _crypto_SHA256_cb_(uint8_t* p_Sha256, uint8_t* p_Data, uint32_t u32_Sz, int cmock_num_calls)
{
	TEST_ASSERT_NOT_NULL(p_Sha256);
	memcpy(p_Sha256, pExpectedSHA256Ptr, SHA256_SIZE);
	TEST_ASSERT_EQUAL_PTR(pExpectedPtr, p_Data);
	TEST_ASSERT_EQUAL(u32ExpectedSz, u32_Sz);
	return CRYPTO_OK;
}
/******************************************************************************/

//#include "unity_fixture.h"
TEST_SETUP(Samples_ImgStorage)
{
	sImgMgrCtx.erase = NULL;
	sImgMgrCtx.write = NULL;
	u32ExpectedSz = 0;
	pExpectedPtr = NULL;
	pExpectedSHA256Ptr = NULL;
}
TEST_TEAR_DOWN(Samples_ImgStorage)
{}

/******************************************************************************/
TEST(Samples_ImgStorage, test_Imgstore_Setup)
{
	uint32_t u32ImgAdd;
	uint8_t ret;

	// check "normal" use
	u32ImgAdd = 0x00000000;
	ret = ImgStore_Setup(u32ImgAdd, _local_write, _local_erase);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_EQUAL(NULL, sImgMgrCtx.erase);
	TEST_ASSERT_NOT_EQUAL(NULL, sImgMgrCtx.write);

	// check unalign address
	u32ImgAdd = 0x0000007;
	ret = ImgStore_Setup(u32ImgAdd, _local_write, _local_erase);
	TEST_ASSERT_EQUAL(1, ret);
	TEST_ASSERT_EQUAL(0, sImgMgrCtx.u32Size);
}

TEST(Samples_ImgStorage, test_Imgstore_Init)
{
	uint16_t u16NbExpectedBlk;
	int8_t ret;

	_fill_FF_bitmap();
	_fill_NON_EMPTY_pend();

	// check too large block nb
	u16NbExpectedBlk = 10000;
	ret = ImgStore_Init(u16NbExpectedBlk);
	TEST_ASSERT_EQUAL(1, ret);

	u16NbExpectedBlk = 100;

	// check pending flag
	ret = ImgStore_Init(u16NbExpectedBlk);
	TEST_ASSERT_EQUAL(1, ret);

	// check erase failed
	ImgStore_SetPending(PENDING_NONE);
	sImgMgrCtx.erase = &_fake_erase;
	ret = ImgStore_Init(u16NbExpectedBlk);
	TEST_ASSERT_EQUAL(-1, ret);

	sImgMgrCtx.erase = NULL;

	// check that all are cleared
	ret = ImgStore_Init(u16NbExpectedBlk);
	TEST_ASSERT_EQUAL(0, sImgMgrCtx.aBitMap[0]);
	TEST_ASSERT_EQUAL(0, sImgMgrCtx.aBitMap[BITMAP_BLOCK_NB-1]);

	TEST_ASSERT_EQUAL(0x8000, sImgMgrCtx.sPendCtrl[0].u16Id);
	TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.sPendBuff[0].aPend[0]);
	TEST_ASSERT_EQUAL(0x8000, sImgMgrCtx.sPendCtrl[PENDING_LINE_NB-1].u16Id);
	TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.sPendBuff[PENDING_LINE_NB-1].aPend[PENDING_LINE_PEND_BYTES-1]);
}

TEST(Samples_ImgStorage, test_Imgstore_IsComplete)
{
	uint16_t u16NbExpectedBlk;
	uint8_t ret;

	u16NbExpectedBlk = 100;
	ImgStore_SetPending(PENDING_NONE);
	ret = ImgStore_Init(u16NbExpectedBlk);
	TEST_ASSERT_EQUAL(0, ret);

	ret = ImgStore_IsComplete();
	TEST_ASSERT_EQUAL(0, ret);

	sImgMgrCtx.u32NbBlk = 99;
	ret = ImgStore_IsComplete();
	TEST_ASSERT_EQUAL(0, ret);

	sImgMgrCtx.u32NbBlk = 100;
	ret = ImgStore_IsComplete();
	TEST_ASSERT_EQUAL(1, ret);
}

TEST(Samples_ImgStorage, test_Imgstore_Verify)
{
	uint32_t u32ImgAdd;
	uint16_t u16ExpectedBlk;
	uint8_t *p;
	uint8_t ret;

	u32ExpectedSz = aLoremIpsum_p1_Sz;
	pExpectedPtr = aLoremIpsum;
	pExpectedSHA256Ptr = aLoremIpsum_p1_Hash;

	// check "normal" use
	u32ImgAdd = (uint32_t)(&aLoremIpsum[0]);
	ret = ImgStore_Setup(u32ImgAdd, _local_write, _local_erase);
	TEST_ASSERT_EQUAL(0,ret);
	sImgMgrCtx.erase = NULL;
	sImgMgrCtx.write = NULL;

	Crypto_SHA256_Stub(_crypto_SHA256_cb_);
	//**********************
	u16ExpectedBlk = aLoremIpsum_p1_Sz / BLOCK_SZ;
	p = aLoremIpsum_p1_Hash;
	ret = ImgStore_Init(u16ExpectedBlk);
	TEST_ASSERT_EQUAL(0, ret);

	// test if address and size are correct
	ret = ImgStore_Verify(p, 4);
	TEST_ASSERT_EQUAL(0,ret);

	// test case when Crypto_SHA256 failed
	Crypto_SHA256_IgnoreAndReturn(CRYPTO_KO);
	ret = ImgStore_Verify(p, 4);
	TEST_ASSERT_EQUAL(1,ret);

	// test case when pHash is NULL
	Crypto_SHA256_IgnoreAndReturn(CRYPTO_OK);
	ret = ImgStore_Verify(NULL, 4);
	TEST_ASSERT_EQUAL(1,ret);
}

TEST(Samples_ImgStorage, test_Imgstore_StoreBlock)
{
#define NB_TEST 4
	uint16_t aExpectedBlk[NB_TEST] =
	{
		aLoremIpsum_p1_Sz / BLOCK_SZ,
		aLoremIpsum_p2_Sz / BLOCK_SZ,
		aLoremIpsum_p3_Sz / BLOCK_SZ,
		aLoremIpsum_p4_Sz / BLOCK_SZ,
	};

	uint8_t *apExpectedHash[NB_TEST] =
	{
		aLoremIpsum_p1_Hash,
		aLoremIpsum_p2_Hash,
		aLoremIpsum_p3_Hash,
		aLoremIpsum_p4_Hash
	};

	uint8_t *pSrc;
	uint8_t *pBuff;
	uint16_t u16ExpectedBlk;
	uint16_t i;
	uint8_t j;
	uint8_t ret;
	char str[50];

// TODO : check "normal" use
#undef IMG_ADRR
#define IMG_ADRR (uint32_t)(aTargetArea)

//#undef IMG_MAX_SZ
//#define IMG_MAX_SZ sizeof(aLoremIpsum)

// #define IMG_ADRR 0x0802C000

	pBuff = (uint8_t*)(IMG_ADRR);

	ret = ImgStore_Setup(IMG_ADRR, _local_write, _local_erase);
	TEST_ASSERT_EQUAL(0,ret);

	sImgMgrCtx.u32Size = IMG_MAX_SZ;

	u16ExpectedBlk = aLoremIpsum_p1_Sz / BLOCK_SZ;
	ret = ImgStore_Init(u16ExpectedBlk);
	TEST_ASSERT_EQUAL(0,ret);
	TEST_ASSERT_EQUAL(0xFF, pBuff[0]);
	TEST_ASSERT_EQUAL(0xFF, pBuff[sImgMgrCtx.u32Size - 1]);

	pExpectedPtr = pBuff;
	Crypto_SHA256_Stub(_crypto_SHA256_cb_);
	for (j = 0 ; j < NB_TEST; j++)
	{
		u16ExpectedBlk = aExpectedBlk[j];
		sprintf(str, "Test %d; Expected blk %d", j, u16ExpectedBlk);
 		ret = ImgStore_Init(u16ExpectedBlk);
		TEST_ASSERT_EQUAL_MESSAGE(0, ret, str);
		for (i = 0; i < u16ExpectedBlk; i++)
		{
			pSrc = &aLoremIpsum[i*BLOCK_SZ];
			ImgStore_StoreBlock(i, pSrc);
		}
		TEST_ASSERT_EQUAL_MEMORY_MESSAGE(aLoremIpsum, pBuff, u16ExpectedBlk*BLOCK_SZ, str);

		ret = ImgStore_IsComplete();
		TEST_ASSERT_EQUAL_MESSAGE(1, ret, str);

		u32ExpectedSz = u16ExpectedBlk * BLOCK_SZ  ;
		pExpectedSHA256Ptr = &(apExpectedHash[j][0]);

		ret = ImgStore_Verify(apExpectedHash[j], 4);
		TEST_ASSERT_EQUAL_MESSAGE(0, ret, str);
	}
#undef IMG_ADRR
}
