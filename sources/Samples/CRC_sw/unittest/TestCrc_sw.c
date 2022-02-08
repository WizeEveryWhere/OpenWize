#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(Samples_CRC_sw);

#include "crc_sw.h"
static uint16_t L2_CRC = 0x2223;

static uint8_t L2_content[70] = {
    0x45, // LF
    0x44, // CF
    0x53, 0x10, // MF
    0x73, 0x03, 0x15, 0x00, 0x00, 0x03,// AF
    0xb4, //ClF (Wize 1.0)
    0x21, // L6Ctrl
    0x01, // L6OprId
    0x00, 0x00, // L6Cpt
    0x00, // L6App
    0x95, 0x98, 0xaa, 0xfa,
    0xe6, 0xb0, 0x2c, 0xa1, 0xda, 0x1b, 0xcd, 0xd2, 0x6a, 0x4f, // 10
    0xda, 0x4d, 0x07, 0x0c, 0x05, 0x9b, 0x54, 0x41, 0xd4, 0xce,
    0x95, 0x33, 0xcf, 0x23, 0xf3, 0xfa, 0x32, 0xda, 0xc2, 0x51,
    0x9e, 0x63, 0xe0, 0xa7, 0xaa, 0x28, 0x0c, 0x27, 0xdc, 0xc3,
    0x3d, 0xf9, 0x93, 0x03, // L6HashKenc
    0x00, 0x00, // L6TStamp
    0xb9, 0x81, // L6HashKmac
    0x22, 0x23 // CRC
};

static uint8_t data_idx = 0;
static uint8_t data_sz = 68;

TEST_SETUP(Samples_CRC_sw)
{

}

TEST_TEAR_DOWN(Samples_CRC_sw)
{

}

TEST(Samples_CRC_sw, test_CRC_Compute_Success)
{
	uint8_t ret, sz;
	uint8_t *p_Buf;
	uint16_t crc, expected;

	p_Buf = &(L2_content[data_idx]);
	sz = data_sz;
	expected = L2_CRC;

	ret = CRC_Compute(p_Buf, sz, &crc);
	TEST_ASSERT_EQUAL(1, ret);
	TEST_ASSERT_EQUAL_UINT16(expected, crc);
}

TEST(Samples_CRC_sw, test_CRC_Compute_NullPointer)
{
	uint8_t ret, sz;
	uint8_t *p_Buf;
	uint16_t crc;

	p_Buf = &(L2_content[data_idx]);
	sz = 0;
	ret = CRC_Compute(p_Buf, sz, &crc);
	TEST_ASSERT_EQUAL(0, ret);

	p_Buf = NULL;
	sz = data_sz;
	ret = CRC_Compute(p_Buf, sz, &crc);
	TEST_ASSERT_EQUAL(0, ret);
}

TEST(Samples_CRC_sw, test_CRC_Check_Success)
{
	uint8_t ret;
	uint16_t crc, expected;

	crc = L2_CRC;
	expected = L2_CRC;
	ret = CRC_Check(expected, crc);
	TEST_ASSERT_EQUAL(1, ret);

	crc = 0xFFFF;
	expected = 0xFFFF;
	ret = CRC_Check(expected, crc);
	TEST_ASSERT_EQUAL(1, ret);

	crc = 0;
	expected = 0;
	ret = CRC_Check(expected, crc);
	TEST_ASSERT_EQUAL(1, ret);
}

TEST(Samples_CRC_sw, test_CRC_Check_Fail)
{
	uint8_t ret;
	uint16_t crc, expected;

	crc = 0xFFFF;
	expected = 0;
	ret = CRC_Check(expected, crc);
	TEST_ASSERT_EQUAL(0, ret);

	crc = 0;
	expected = 0xFFFF;
	ret = CRC_Check(expected, crc);
	TEST_ASSERT_EQUAL(0, ret);
}
