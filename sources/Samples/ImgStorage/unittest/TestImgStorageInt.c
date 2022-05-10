#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(Samples_ImgStorageInt);

#include "img_storage_private.h"

/******************************************************************************/

extern struct img_mgr_ctx_s sImgMgrCtx;

/******************************************************************************/

//#include "unity_fixture.h"
TEST_SETUP(Samples_ImgStorageInt)
{}

TEST_TEAR_DOWN(Samples_ImgStorageInt)
{}

/******************************************************************************/

extern uint8_t _get_bitmap_line(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id);
extern uint8_t _get_bitmap_bit(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id);
extern void _set_bitmap(struct img_mgr_ctx_s *pCtx, uint16_t u16_Id);
extern void _clr_bitmap(struct img_mgr_ctx_s *pCtx);
extern void _clr_pending(struct img_mgr_ctx_s *pCtx);

TEST(Samples_ImgStorageInt, test_ImgstoreInt_ClrBitmap)
{
#define BLK_ID_MAX 5
	uint16_t i;
	for (i = 0; i < BLK_ID_MAX; i++)
	{
		sImgMgrCtx.aBitMap[i] = 0xFF;
	}

	_clr_bitmap(&sImgMgrCtx);

	for (i = 0; i < BLK_ID_MAX; i++)
	{
		TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.aBitMap[i]);
	}

	TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.aBitMap[BITMAP_BLOCK_NB-1]);
#undef BLK_ID_MAX
}

TEST(Samples_ImgStorageInt, test_ImgstoreInt_SetBitmap)
{
#define BLK_ID_MAX 40 // 5*8
	uint16_t id, i;
	uint8_t expected;
	_clr_bitmap(&sImgMgrCtx);

	id = 1;
	expected = 0b10000000 >> 1;
	_set_bitmap(&sImgMgrCtx, id);
	TEST_ASSERT_EQUAL(expected, sImgMgrCtx.aBitMap[0]);

	id = 12;
	expected = 0b10000000 >> 4;
	_set_bitmap(&sImgMgrCtx, 12);
	TEST_ASSERT_EQUAL(expected, sImgMgrCtx.aBitMap[1]);

	for (id = 0; id < BLK_ID_MAX; id++)
	{
		_set_bitmap(&sImgMgrCtx, id);
	}
	for (i = 0; i < BLK_ID_MAX/8; i++)
	{
		TEST_ASSERT_EQUAL(0xFF, sImgMgrCtx.aBitMap[i]);
	}
	TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.aBitMap[i]);
#undef BLK_ID_MAX
}

TEST(Samples_ImgStorageInt, test_ImgstoreInt_GetLineBitmap)
{
#define BLK_ID 42 // 5*8
	uint8_t ret;
	_clr_bitmap(&sImgMgrCtx);

	sImgMgrCtx.aBitMap[5] = 0xAA;
	ret = _get_bitmap_line(&sImgMgrCtx, BLK_ID);
	TEST_ASSERT_EQUAL(0xAA, ret);
#undef BLK_ID
}

TEST(Samples_ImgStorageInt, test_ImgstoreInt_GetBitmap)
{
#define BLK_ID 18
	uint8_t ret;
	uint8_t expected;
	_clr_bitmap(&sImgMgrCtx);

	_set_bitmap(&sImgMgrCtx, BLK_ID);
	ret = _get_bitmap_bit(&sImgMgrCtx, BLK_ID);
	TEST_ASSERT_TRUE(ret);

	expected = 0b10000000 >> 2;
	ret = _get_bitmap_line(&sImgMgrCtx, BLK_ID);
	TEST_ASSERT_EQUAL(expected, ret);
#undef BLK_ID
}

TEST(Samples_ImgStorageInt, test_ImgstoreInt_ClrPending)
{
	sImgMgrCtx.sPendCtrl[0].u16Id = 0xFFFF;
	sImgMgrCtx.sPendBuff[0].aPend[0] = 0xFF;

	sImgMgrCtx.sPendCtrl[PENDING_LINE_NB-1].u16Id = 0xFFFF;
	sImgMgrCtx.sPendBuff[PENDING_LINE_NB-1].aPend[PENDING_LINE_PEND_BYTES-1] = 0xFF;
	_clr_pending(&sImgMgrCtx);

	TEST_ASSERT_EQUAL(0x8000, sImgMgrCtx.sPendCtrl[0].u16Id);
	TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.sPendBuff[0].aPend[0]);
	TEST_ASSERT_EQUAL(0x8000, sImgMgrCtx.sPendCtrl[PENDING_LINE_NB-1].u16Id);
	TEST_ASSERT_EQUAL(0x00, sImgMgrCtx.sPendBuff[PENDING_LINE_NB-1].aPend[PENDING_LINE_PEND_BYTES-1]);
}
