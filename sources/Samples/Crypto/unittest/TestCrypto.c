#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(Samples_Crypto);

#include "crypto.h"
#include "key_priv.h"

#ifndef USE_GOLDEN_KEY
// keys
KEY_STORE key_s _a_Key_[KEY_MAX_NB] = {
		[1].key = {0xae, 0x68, 0x52, 0xf8, 0x12, 0x10, 0x67, 0xcc, 0x4b, 0xf7, 0xa5, 0x76, 0x55, 0x77, 0xf3, 0x9e}, // key_16
		[2].key = {0x7e, 0x24, 0x06, 0x78, 0x17, 0xfa, 0xe0, 0xd7, 0x43, 0xd6, 0xce, 0x1f, 0x32, 0x53, 0x91, 0x63}, // key_32
		[3].key = {0x76, 0x91, 0xbe, 0x03, 0x5e, 0x50, 0x20, 0xa8, 0xac, 0x6e, 0x61, 0x85, 0x29, 0xf9, 0xa0, 0xdc}, // key_36
		[10].key = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c}, // kenc for hash test
		[KEY_MAC_ID].key = {0x16, 0x03, 0x3c, 0x93, 0xf6, 0xcf, 0x97, 0x3e, 0x0a, 0xfb, 0x0e, 0x23, 0xf6, 0xc8, 0xa3, 0xc7 }, // kmac for hash test
};
#endif

// encrypt/decrypt test
static uint8_t plaintext_16 [16] = { 0x53, 0x69, 0x6e, 0x67, 0x6c, 0x65, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x20, 0x6d, 0x73, 0x67 };
static uint8_t ciphertext_16[16] = { 0xe4, 0x09, 0x5d, 0x4f, 0xb7, 0xa7, 0xb3, 0x79, 0x2d, 0x61, 0x75, 0xa3, 0x26, 0x13, 0x11, 0xb8 };
static uint8_t ctr_16             [16] = { 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

static uint8_t plaintext_32 [32] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
static uint8_t ciphertext_32[32] = { 0x51, 0x04, 0xa1, 0x06, 0x16, 0x8a, 0x72, 0xd9, 0x79, 0x0d, 0x41, 0xee, 0x8e, 0xda, 0xd3, 0x88, 0xeb, 0x2e, 0x1e, 0xfc, 0x46, 0xda, 0x57, 0xc8, 0xfc, 0xe6, 0x30, 0xdf, 0x91, 0x41, 0xbe, 0x28 };
static uint8_t ctr_32             [16] = { 0x00, 0x6c, 0xb6, 0xdb, 0xc0, 0x54, 0x3b, 0x59, 0xda, 0x48, 0xd9, 0x0b, 0x00, 0x00, 0x00, 0x01 };

static uint8_t plaintext_36 [36] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23 };
static uint8_t ciphertext_36[36] = { 0xc1, 0xcf, 0x48, 0xa8, 0x9f, 0x2f, 0xfd, 0xd9, 0xcf, 0x46, 0x52, 0xe9, 0xef, 0xdb, 0x72, 0xd7, 0x45, 0x40, 0xa4, 0x2b, 0xde, 0x6d, 0x78, 0x36, 0xd5, 0x9a, 0x5c, 0xea, 0xae, 0xf3, 0x10, 0x53, 0x25, 0xb2, 0x07, 0x2f };
static uint8_t ctr_36             [16] = { 0x00, 0xe0, 0x01, 0x7b, 0x27, 0x77, 0x7f, 0x3f, 0x4a, 0x17, 0x86, 0xf0, 0x00, 0x00, 0x00, 0x01 };

static uint8_t ciphertextFail_36[36] = { 0xaa, 0xbb, 0xcc, 0xa8, 0x9f, 0x2f, 0xfd, 0xd9, 0xcf, 0x46, 0x52, 0xe9, 0xef, 0xdb, 0x72, 0xd7, 0x45, 0x40, 0xa4, 0x2b, 0xde, 0x6d, 0x78, 0x36, 0xd5, 0x9a, 0x5c, 0xea, 0xae, 0xf3, 0x10, 0x53, 0x25, 0xb2, 0x07, 0x2f };

// hash test

static const uint8_t L2_content[70] = {
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

static const uint8_t L7_idx = 16;
static const uint8_t L7_sz = 44;
static const uint8_t L6_idx = 11;
static const uint8_t L6_sz = 55;

static const uint8_t CTR_kenc[16] = {
    0x53, 0x10, // MF
    0x73, 0x03, 0x15, 0x00, 0x00, 0x03, // AF
    0x00, 0x00, // L6Cpt
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
};

static const uint8_t CTR_kmac[16] = {
    0x53, 0x10, // MF
    0x73, 0x03, 0x15, 0x00, 0x00, 0x03, // AF
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
};

static const uint8_t L6_HashKenc[16] = {
   0x3d, 0xf9, 0x93, 0x03, 0xfc, 0xf5, 0x90, 0x0d,
   0xd0, 0x46, 0xbd, 0xda, 0x7e, 0x27, 0xf7, 0x43
};

static const uint8_t L6_HashKmac[16] = {
   0xb9, 0x81, 0x5b, 0x65, 0x74, 0x59, 0x19, 0x06,
   0xe3, 0xb8, 0x36, 0x86, 0xc0, 0xa0, 0xa9, 0x4e
};

static const uint8_t keyId_msg16 = 1;
static const uint8_t keyId_msg32 = 2;
static const uint8_t keyId_msg36 = 3;
static const uint8_t keyId_hashkenc = 10;
static const uint8_t keyId_hashkmac = KEY_MAC_ID;


TEST_SETUP(Samples_Crypto)
{

}

TEST_TEAR_DOWN(Samples_Crypto)
{

}

static void check_result(const void *expected, size_t expectedlen,
                   const void *computed, size_t computedlen)
{
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expectedlen, computedlen, "The length of the computed buffer does not match the expected length.");
	TEST_ASSERT_EQUAL_MEMORY_MESSAGE(computed, expected, computedlen, "The computed buffer does not match the expected one");
}

static void check_str_not_equal(const void *expected, size_t expectedlen,
                   const void *computed, size_t computedlen)
{
	int ret = 0;
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expectedlen, computedlen, "The length of the computed buffer does not match the expected length.");
	ret = memcmp(expected, computed, computedlen);
	TEST_ASSERT_TRUE_MESSAGE(ret, "The computed buffer does match the expected one");
}

TEST(Samples_Crypto, test_Crypto_Encrypt16_Success)
{
	uint8_t *p_In, *p_Out, *p_Expected;
	uint8_t ret, in_sz, expected_sz, keyId;
	uint8_t buff[256];
	uint8_t ctr[CTR_SIZE];

	memcpy(ctr, ctr_16, CTR_SIZE);
	p_Out = buff;

	// test_Crypto_16
	keyId = keyId_msg16;

	p_In = plaintext_16;
	in_sz = sizeof(plaintext_16);

	expected_sz = sizeof(ciphertext_16);
	p_Expected = ciphertext_16;

	ret = Crypto_Encrypt(p_Out, p_In, in_sz, ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	check_result(p_Expected, expected_sz, p_Out, in_sz);
}

TEST(Samples_Crypto, test_Crypto_Encrypt32_Success)
{
	uint8_t *p_In, *p_Out, *p_Expected;
	uint8_t ret, in_sz, expected_sz, keyId;
	uint8_t buff[256];
	uint8_t ctr[CTR_SIZE];

	memcpy(ctr, ctr_32, CTR_SIZE);
	p_Out = buff;

	// test_Crypto_32
	keyId = keyId_msg32;
	p_In = plaintext_32;
	in_sz = sizeof(plaintext_32);

	expected_sz = sizeof(ciphertext_32);
	p_Expected = ciphertext_32;

	ret = Crypto_Encrypt(p_Out, p_In, in_sz, ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	check_result(p_Expected, expected_sz, p_Out, in_sz);
}

TEST(Samples_Crypto, test_Crypto_Encrypt36_Success)
{
	uint8_t *p_In, *p_Out, *p_Expected;
	uint8_t ret, in_sz, expected_sz, keyId;
	uint8_t buff[256];
	uint8_t ctr[CTR_SIZE];

	memcpy(ctr, ctr_36, CTR_SIZE);
	p_Out = buff;

	// test_Crypto_36
	keyId = keyId_msg36;
	p_In = plaintext_36;
	in_sz = sizeof(plaintext_36);

	expected_sz = sizeof(ciphertext_36);
	p_Expected = ciphertext_36;

	ret = Crypto_Encrypt(p_Out, p_In, in_sz, ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	check_result(p_Expected, expected_sz, p_Out, in_sz);
}

TEST(Samples_Crypto, test_Crypto_Encrypt_Mismatch)
{
	uint8_t *p_In, *p_Out, *p_Expected;
	uint8_t ret, in_sz, expected_sz, keyId;
	uint8_t buff[256];
	uint8_t ctr[CTR_SIZE];

	memcpy(ctr, ctr_36, CTR_SIZE);
	p_Out = buff;

	// test_Crypto_36
	keyId = keyId_msg36;

	p_In = plaintext_36;
	in_sz = sizeof(plaintext_36);

	expected_sz = sizeof(ciphertextFail_36);
	p_Expected = ciphertextFail_36;

	ret = Crypto_Encrypt(p_Out, p_In, in_sz, ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	check_str_not_equal(p_Expected, expected_sz, p_Out, in_sz);
}

TEST(Samples_Crypto, test_Crypto_Encrypt_Fail)
{
	// should never !!!
	/*
	uint8_t ret, sz, keyId;
	uint8_t buff[256];

	keyId = keyId_msg16;
	sz = 32;
	ret = Crypto_Encrypt(buff, buff, sz, NULL, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_KO, ret);
	*/
}

TEST(Samples_Crypto, test_Crypto_Encrypt_BadKey)
{
	uint8_t ret, sz, keyId;
	uint8_t buff[256];

	keyId = KEY_MAX_NB+1;
	sz = 32;
	ret = Crypto_Encrypt(buff, buff, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_KID_UNK_ERR, ret);
}

TEST(Samples_Crypto, test_Crypto_Encrypt_Key0)
{

	uint8_t *p_In, *p_Out, *p_Expected;
	uint8_t ret, in_sz, expected_sz, keyId;
	uint8_t buff[256];
	uint8_t ctr[CTR_SIZE];

	memcpy(ctr, ctr_16, CTR_SIZE);
	p_Out = buff;

	// test_Crypto_16
	keyId = 0;
	p_In = plaintext_16;
	in_sz = sizeof(plaintext_16);

	expected_sz = sizeof(plaintext_16);
	p_Expected = plaintext_16;

	ret = Crypto_Encrypt(p_Out, p_In, in_sz, ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	check_result(p_Expected, expected_sz, p_Out, in_sz);
}

TEST(Samples_Crypto, test_Crypto_Encrypt_NullPointer)
{
	uint8_t ret, sz, keyId;
	uint8_t buff[256];

	keyId = keyId_msg16;
	sz = 0;
	ret = Crypto_Encrypt(buff, buff, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_INT_NULL_ERR, ret);

	sz = 32;
	ret = Crypto_Encrypt(NULL, buff, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_INT_NULL_ERR, ret);
	ret = Crypto_Encrypt(buff, NULL, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_INT_NULL_ERR, ret);
}

TEST(Samples_Crypto, test_Crypto_Decrypt16_Success)
{
	uint8_t *p_In, *p_Out, *p_Expected;
	uint8_t ret, in_sz, expected_sz, keyId;
	uint8_t buff[256];
	uint8_t ctr[CTR_SIZE];

	memcpy(ctr, ctr_16, CTR_SIZE);
	p_Out = buff;

	// test_Crypto_16
	keyId = keyId_msg16;
	p_In = ciphertext_16;
	in_sz = sizeof(ciphertext_16);

	expected_sz = sizeof(plaintext_16);
	p_Expected = plaintext_16;

	ret = Crypto_Decrypt(p_Out, p_In, in_sz, ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	check_result(p_Expected, expected_sz, p_Out, in_sz);
}

TEST(Samples_Crypto, test_Crypto_AES128_CMAC_Kenc_Success){
	uint8_t *p_Msg;
	uint8_t *p_Ctr;
	uint8_t *p_Expected;
	uint8_t ret, sz, keyId;
	uint8_t p_Hash[CTR_SIZE];

	p_Msg = (uint8_t *)(&L2_content[L7_idx]);
	sz = L7_sz;

	p_Ctr = (uint8_t *)CTR_kenc;
	keyId = keyId_hashkenc;
	ret = Crypto_AES128_CMAC(p_Hash, p_Msg, sz, p_Ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	p_Expected = (uint8_t *)L6_HashKenc;
	check_result(p_Expected, CTR_SIZE, p_Hash, CTR_SIZE);
}

TEST(Samples_Crypto, test_Crypto_AES128_CMAC_Kmac_Success){
	uint8_t *p_Msg;
	uint8_t *p_Ctr;
	uint8_t *p_Expected;
	uint8_t ret, sz, keyId;
	uint8_t p_Hash[CTR_SIZE];

	p_Msg = (uint8_t *)(&L2_content[L6_idx]);
	sz = L6_sz;

	p_Ctr = (uint8_t *)CTR_kmac;
	keyId = keyId_hashkmac;
	ret = Crypto_AES128_CMAC(p_Hash, p_Msg, sz, p_Ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	p_Expected = (uint8_t *)L6_HashKmac;
	check_result(p_Expected, CTR_SIZE, p_Hash, CTR_SIZE);
}

TEST(Samples_Crypto, test_Crypto_AES128_CMAC_Mismatch)
{
	uint8_t *p_Msg;
	uint8_t *p_Ctr;
	uint8_t *p_Expected;
	uint8_t ret, sz, keyId;
	uint8_t p_Hash[CTR_SIZE];

	p_Msg = (uint8_t *)(&L2_content[L6_idx]);
	sz = L6_sz-5;

	p_Ctr = (uint8_t *)CTR_kmac;
	keyId = keyId_hashkmac;
	ret = Crypto_AES128_CMAC(p_Hash, p_Msg, sz, p_Ctr, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_OK, ret);
	p_Expected = (uint8_t *)L6_HashKmac;
	check_str_not_equal(p_Expected, CTR_SIZE, p_Hash, CTR_SIZE);
}

TEST(Samples_Crypto, test_Crypto_AES128_CMAC_Fail)
{
 // should never !!!
}

TEST(Samples_Crypto, test_Crypto_AES128_CMAC_BadKey)
{
	uint8_t ret, sz, keyId;
	uint8_t buff[256];

	sz = 32;
	keyId = KEY_MAX_NB+1;
	ret = Crypto_AES128_CMAC(buff, buff, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_KID_UNK_ERR, ret);
}

TEST(Samples_Crypto, test_Crypto_AES128_CMAC_NullPointer)
{
	uint8_t ret, sz, keyId;
	uint8_t buff[256];

	keyId = keyId_hashkenc;
	sz = 0;
	ret = Crypto_AES128_CMAC(buff, buff, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_INT_NULL_ERR, ret);

	sz = 32;
	ret = Crypto_AES128_CMAC(NULL, buff, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_INT_NULL_ERR, ret);
	ret = Crypto_AES128_CMAC(buff, NULL, sz, buff, keyId);
	TEST_ASSERT_EQUAL(CRYPTO_INT_NULL_ERR, ret);

}
