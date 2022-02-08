#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(Samples_ReedSolomon);

#include "rs.h"

#define SHOW_GENERATE_TABLES
#undef SHOW_GENERATE_TABLES

#define SHOW_DATA
#undef SHOW_DATA

static uint8_t p_Data[RS_MESSAGE_SZ + RS_PARITY_SZ];
static uint8_t p_Recd[RS_MESSAGE_SZ + RS_PARITY_SZ];

static uint8_t data[RS_MESSAGE_SZ];
static uint8_t parity[RS_PARITY_SZ];

static uint8_t check_parity(void);
static uint8_t check_data(void);
static void prepare_data(void);
void print_recd(void);

TEST_SETUP(Samples_ReedSolomon)
{
	RS_Init();
	prepare_data();
}

TEST_TEAR_DOWN(Samples_ReedSolomon)
{

}

static void prepare_data(void)
{
	int32_t i;
	uint8_t parity_sz = sizeof(parity);

	// prepare original data message and the recd part for message
	for(i = 0; i< RS_MESSAGE_SZ; i++) {
		data[i] = 255-i;
	}
	data[223-1] = 0;
	for(i = 0; i< RS_MESSAGE_SZ; i++) {
		p_Recd[i] = data[i];
	}

	memset(parity, 0, parity_sz);

	// encode data[] to produce parity in parity[].  Data input and parity output
	//	is in polynomial form
	RS_Encode(data, parity);

	// get the computed parity and set it into p_Recd, so that :
	for (i = 0; i < (nn-kk); i++) {
		p_Recd[i+kk] = parity[i];
	}
	// from now :
	// p_Recd[0 to kk] = data [0 to kk]
	// p_Recd[nn-kk to nn] = parity [0 to nn-kk]
}

static uint8_t check_parity(void)
{
	int32_t i;
	uint8_t ret = 1;
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	for (i = 0; i < RS_PARITY_SZ; i++)
	{
		if(parity[i] != p_Recd[i+kk])
		{
			ret = 0;
			break;
		}
	}
	return ret;
}

static uint8_t check_data(void)
{
	int32_t i;
	uint8_t ret = 1;
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	for (i = 0; i < RS_MESSAGE_SZ; i++)
	{
		if(data[i] != p_Recd[i])
		{
			ret = 0;
			break;
		}
	}
#ifdef SHOW_DATA
	print_recd();
#endif // SHOW_DATA
	return ret;
}

void print_recd(void){
	int32_t i;
	//uint8_t msg_sz, parity_sz;
	//msg_sz = RS_GetMsgSize();
	//parity_sz = RS_GetParitySize();

	printf("Results for Reed-Solomon code ((msg_sz + parity_sz)=%3d, msg_sz=%3d, tt= %3d(parity/2))\n\n",(RS_MESSAGE_SZ + RS_PARITY_SZ),RS_MESSAGE_SZ,tt) ;
	printf("  i  data[i]   recd[i](decoded)   (data, recd in polynomial form)\n");

	printf("**** Data \n") ;
	for (i = 0; i < RS_MESSAGE_SZ; i++)
	{
		printf("%3ld    %3d      %3d\n",i, data[i], p_Recd[i]) ;
		//if ( i == 50) { break; }
	}
	printf("**** Parity \n") ;
	for (i = 0; i < RS_PARITY_SZ; i++)
	{
		printf("%3ld    %3d      %3d\n",i, parity[i], p_Recd[i+RS_MESSAGE_SZ] ) ;
		if ( i == 50) { break; }
	}

	printf(".........................\n") ;
}

//==============================================================================
TEST(Samples_ReedSolomon, test_RS_GenerateTablesCheck){
	//const uint8_t *p_gg, *p_alpha_to;
	//const int16_t *p_index_of;
	uint32_t u32_alphaOfSz, u32_indexOfSz;

	//p_gg = RS_GetGG_ptr(&u32_ggSz);
	RS_GetAlphaOf_ptr(&u32_alphaOfSz);
	RS_GetIndexOf_ptr(&u32_indexOfSz);

	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, (u32_alphaOfSz == u32_indexOfSz), "indexOf and alphaOf Sizes don't match.");

#ifdef SHOW_GENERATE_TABLES
	printf("Look-up tables for GF(2**%2d)\n",u32_indexOfSz) ;
	printf("  i   alpha_to[i]  index_of[i]\n") ;
	for (i = 0; i < u32_indexOfSz; i++){
		printf("%3d      %3d          %3d\n",i,p_alpha_to[i],p_index_of[i]) ;
	}
	printf("\n\n") ;
#endif
}

// No error
TEST(Samples_ReedSolomon, test_RS_Decode_0ErrorMessage)
{
	uint8_t ret = 0;

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial form

	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// 1 error on msg
TEST(Samples_ReedSolomon, test_RS_Decode_1ErrorMessage)
{
	uint8_t ret = 0;

	p_Recd[2] = 2;

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial form
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// 1 error on parity
TEST(Samples_ReedSolomon, test_RS_Decode_1ErrorParity)
{
	uint8_t ret = 0;

	p_Recd[RS_MESSAGE_SZ+2] = RS_MESSAGE_SZ+2;

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial form
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// 16 errors on msg
TEST(Samples_ReedSolomon, test_RS_Decode_16ErrorMessage)
{
	uint8_t ret = 0;
	int32_t i;
	for(i = 0; i< 16; i++) {
		p_Recd[i*2] = i*2;
	}

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial form

	//print_recd();

	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// 16 errors on parity
TEST(Samples_ReedSolomon, test_RS_Decode_16ErrorParity)
{
	uint8_t ret = 0;
	int32_t i;
	for(i = 0; i< 16; i++) {
		p_Recd[RS_MESSAGE_SZ + i*2] = i;
	}
	ret = RS_Decode(p_Recd); // data[] is returned in polynomial form
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// 16 errors on msg and parity
TEST(Samples_ReedSolomon, test_RS_Decode_16ErrorMessageAndParity)
{
	uint8_t ret = 0;
	int32_t i;
	for(i = 0; i< 8; i++) {
		p_Recd[i*2 + 1] = i*2 + 1;
	}
	for(i = 0; i< 8; i++) {
		p_Recd[RS_MESSAGE_SZ + i*2 + 1] = i + 1;
	}

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial form
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// more than 16 errors
TEST(Samples_ReedSolomon, test_RS_Decode_17ErrorMessageAndParity)
{
	uint8_t ret = 0;
	int32_t i;
	for(i = 0; i< 8; i++) {
		p_Recd[i*2 + 1] = i*2 + 1;
	}
	for(i = 0; i< 8; i++) {
		p_Recd[RS_MESSAGE_SZ + i*2 + 1] = i + 1;
	}

	p_Recd[RS_MESSAGE_SZ + 30] = 30;

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial formm
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, check_data(), "RS Data doesn't match.");
}


static const uint8_t L2_FRM_DOWNLOAD[256] = {
	0xFF, 0x00, 0x00, 0x01, // L2 header
	0x00, 0x00, 0x00, 0x01, // L6 header
	0xb9, 0x29, 0xb7, 0x5e, 0xa4, 0x74, 0xa3, 0x10, 0x43, 0x07,
	0x99, 0x84, 0xe6, 0x4c, 0x84, 0x0f, 0xa5, 0xee, 0xd3, 0xdd,
	0x20, 0x1c, 0x90, 0x93, 0xde, 0xd6, 0x4b, 0xc7, 0x18, 0x4b,
	0x75, 0x70, 0x12, 0xb6, 0x7d, 0x09, 0x08, 0xae, 0x72, 0x00,
	0xd6, 0xcc, 0x79, 0xfe, 0x29, 0xf4, 0x86, 0x78, 0x5b, 0x6f,
	0xe3, 0x9d, 0x27, 0x8a, 0x6c, 0xac, 0x67, 0x84, 0x92, 0xf9,
	0x2b, 0x44, 0x07, 0xae, 0x8a, 0xd1, 0x54, 0x73, 0x77, 0x6d,
	0x44, 0xfc, 0xa8, 0xb2, 0xce, 0x5e, 0xef, 0x26, 0x30, 0xe1,
	0x9a, 0x0c, 0xef, 0x5d, 0xbf, 0xf2, 0xfd, 0x4c, 0x3b, 0x90,
	0xa9, 0x10, 0x77, 0xf0, 0x20, 0xe7, 0x4f, 0xde, 0xa7, 0x6f,
	0xc9, 0x90, 0x38, 0x57, 0x65, 0x25, 0x8a, 0xbd, 0xe9, 0x30,
	0x06, 0x8c, 0xb1, 0xdf, 0x81, 0x48, 0x8b, 0x66, 0x9d, 0xfa,
	0x1b, 0xf3, 0xc4, 0xd4, 0x6e, 0xbe, 0x36, 0xac, 0xc9, 0x4b,
	0x7e, 0xe9, 0x2a, 0x8a, 0xba, 0x29, 0xf3, 0xc4, 0x8a, 0x73,
	0x83, 0x4b, 0x02, 0x22, 0x16, 0x50, 0xdd, 0xfd, 0xdb, 0xfd,
	0xbf, 0xfa, 0xfe, 0x63, 0x6e, 0x2e, 0x96, 0x8e, 0xb7, 0x86,
	0xc5, 0x11, 0xdd, 0x42, 0x8c, 0xb9, 0x10, 0x05, 0x82, 0x76,
	0x3f, 0x77, 0x26, 0xeb, 0x35, 0x30, 0x2a, 0x53, 0x81, 0x2e,
	0x50, 0x13, 0xd7, 0x45, 0x5c, 0xad, 0x1f, 0x8d, 0x56, 0x28,
	0xa8, 0x87, 0x00, 0x1e, 0x9f, 0x73, 0x19, 0xb2, 0x13, 0x5b,
	0xd2, 0x56, 0xe3, 0x32, 0xdc, 0x07, 0xcb, 0xc4, 0x37, 0x90,
	0xca, 0xfa, 0x05, 0x1c,
	// L6 footer
	0x7e, 0xf4,
	0x1f, 0x3f, 0xa6, 0x9b, 0x52, 0x97, 0x86, 0xab,
	0x1b, 0x15, 0xd4, 0x16, 0xa4, 0xde, 0xce, 0xc2,
	0x62, 0xe2, 0xa5, 0x8a, 0x8d, 0x0f,	0x6c, 0x95,
	0x2b, 0x1f, 0x29, 0xf0, 0x7c, 0xc2, 0x04, 0x03, // L2 footer
 };

// pseudo-real frame
TEST(Samples_ReedSolomon, test_RS_Decode_PseudoRealFrame)
{
	uint8_t ret = 0;
	int32_t i;

	for(i = 0; i< RS_MESSAGE_SZ; i++) {
		data[i] = L2_FRM_DOWNLOAD[i+1];
	}

	for (i = 0; i < RS_PARITY_SZ; i++) {
		parity[i] = L2_FRM_DOWNLOAD[i+1+RS_MESSAGE_SZ];
	}

	for (i = 0; i < nn; i++) {
		p_Recd[i] = L2_FRM_DOWNLOAD[i+1];
	}

	ret = RS_Decode(p_Recd); // data[] is returned in polynomial formm
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}

// pseudo-real frame with error
TEST(Samples_ReedSolomon, test_RS_Decode_PseudoRealFrameWithError)
{
	uint8_t ret = 0;
	int32_t i;

	for(i = 0; i< RS_MESSAGE_SZ; i++) {
		data[i] = L2_FRM_DOWNLOAD[i+1];
	}

	for (i = 0; i < RS_PARITY_SZ; i++) {
		parity[i] = L2_FRM_DOWNLOAD[i+1+RS_MESSAGE_SZ];
	}

	for (i = 0; i < nn; i++) {
		p_Recd[i] = L2_FRM_DOWNLOAD[i+1];
	}

	p_Recd[15] = p_Recd[15] +2;


	ret = RS_Decode(p_Recd); // data[] is returned in polynomial formm
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, ret, "RS Decode error correction.");
	// on compare le RS dans parity[ ] avec les 32 premiers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_parity(), "RS Parity doesn't match.");
	// on compare les 223 octets de data[ ] avec les 223 derniers octets de recd[]
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, check_data(), "RS Data doesn't match.");
}
