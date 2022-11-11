#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreProto);

#include "proto_api.h"
#include "proto_private.h"

#include "mock_crc_sw.h"
#include "mock_crypto.h"
#include "mock_rs.h"

uint8_t aBuff[255];
uint8_t aData[255];

static struct proto_ctx_s sCtx =
{
	.aDeviceAddr = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02 },
	.aDeviceManufID = {0xFE, 0xDC},
	.sProtoConfig =
	{
		.DwnId = {1, 2, 3},
		.AppInst = L6APP_INST,
		.AppAdm = L6APP_ADM,
		.AppData = 0xAA,
	},
};

static net_msg_t sNetMsg =
{
	.pData = aData,
	.u32Epoch = 0,
	.u16Id = 0,
	.u8Size = 0,
	.u8Type = 0,
	.u8Rssi = 0,
	.u8KeyId = 0,
	.u16Tstamp = 0,
	.Option = 0
};

uint8_t aHash[L6_HASH_KENC_SZ] = {0xC0, 0xCA, 0xBA, 0xD0};
uint8_t aCrc[CRC_SZ] = {0xCA, 0xD0};
/******************************************************************************/
//
l2_exch_header_t *pL2h;
l6_exch_header_t *pL6h;
l6_exch_footer_t *pL6f;
l2_exch_footer_t *pL2f;
l2_down_header_t *pL2h_dwn;
l2_down_footer_t *pL2f_dwn;
l6_down_header_t *pL6h_dwn;
l6_down_footer_t *pL6f_dwn;

uint8_t l_size;

static void _set_exch_buffer_ptrs_(uint8_t u8Size)
{
    uint8_t l6_start, l7_start, l6_end, l2_end;

    l6_start = sizeof(l2_exch_header_t) + 1;
    l7_start = l6_start + sizeof(l6_exch_header_t);
    l6_end = l7_start + u8Size;
    l2_end = l6_end + sizeof(l6_exch_footer_t);
    l_size = l6_end - 1;

    pL2h = (l2_exch_header_t*)(&(aBuff[1]));
    pL6h = (l6_exch_header_t*)(&(aBuff[l6_start]));
    pL6f = (l6_exch_footer_t*)(&(aBuff[l6_end]));
    pL2f = (l2_exch_footer_t*)(&(aBuff[l2_end]));
}

static void _set_dwn_buffer_ptrs_(uint8_t u8Size)
{
	uint8_t l2_end, l6_start, l6_end, l7_start;

    l6_start = sizeof(l2_down_header_t) +1;
    l2_end = u8Size - sizeof(l2_down_footer_t) +1;
    l6_end = l2_end - sizeof(l6_down_footer_t);
    pL2h_dwn = (l2_down_header_t*)(&(aBuff[1]));
    pL2f_dwn = (l2_down_footer_t*)(&(aBuff[l2_end]));
    pL6h_dwn = (l6_down_header_t*)(&(aBuff[l6_start]));
    pL6f_dwn = (l6_down_footer_t*)(&(aBuff[l6_end]));

    l7_start = l6_start + sizeof(l6_down_header_t);
    l_size = l6_end - l7_start;
}

static void _fill_dwn_buffer_ptrs_(uint8_t u8Size)
{
	_set_dwn_buffer_ptrs_(u8Size);

	memcpy(pL2h_dwn->L2DownId, sCtx.sProtoConfig.DwnId, L2DWNID_SZ);
	pL6h_dwn->L6DownVer = L6_DOWNLOAD_VER;
	memcpy(pL6f_dwn->L6HashLog, aHash, L6_HASH_KLOG_SZ);
	pL6h_dwn->L6DownBnum[0] = 0;
	pL6h_dwn->L6DownBnum[1] = 1;
	pL6h_dwn->L6DownBnum[2] = 1;
}

static void _fill_exch_buffer_ptrs_(uint8_t u8Size)
{
	_set_exch_buffer_ptrs_(u8Size);

	memcpy(pL2h->Afield, sCtx.aDeviceAddr, AFIELD_SZ);
	memcpy(pL2h->Mfield, sCtx.aDeviceManufID, MFIELD_SZ);
	pL2h->Cifield = WIZE_PROTO_ID;
	pL6h->L6Ctrl_b.KEYSEL = 1;
	pL6h->L6Ctrl_b.WTS  = 1;
	pL6h->L6Ctrl_b.VERS = L6VERS;
	*((uint16_t*)pL6h->L6Cpt) = 0xBEEF;
	pL6h->L6NetwId = sCtx.sProtoConfig.u8NetId;
	pL2h->Cfield = COMMAND;

	memcpy( pL6f->L6HashKenc, aHash, L6_HASH_KENC_SZ);
	memcpy( pL6f->L6HKmac, aHash, L6_HASH_KMAC_SZ);
	memcpy( pL2f->Crc, aCrc, CRC_SZ);

	aBuff[0] =(uint8_t*)( (uint8_t*)pL2f - (uint8_t*)pL2h ) + 2;

	sCtx.u8Size = aBuff[0];
}

static void _check_exch_other_content_(uint8_t u8Size)
{
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// Check AFIELD
	TEST_ASSERT_EQUAL_MEMORY( sCtx.aDeviceAddr, pL2h->Afield, AFIELD_SZ);
	// Check MFIELD
	TEST_ASSERT_EQUAL_MEMORY( sCtx.aDeviceManufID, pL2h->Mfield, MFIELD_SZ);
	// Check CIFIELD
	TEST_ASSERT_EQUAL( WIZE_PROTO_ID, pL2h->Cifield);
	// Check HASH KENC
	TEST_ASSERT_EQUAL_MEMORY( aHash, pL6f->L6HashKenc, L6_HASH_KENC_SZ);
	// Check HASH KMAC
	TEST_ASSERT_EQUAL_MEMORY( aHash, pL6f->L6HKmac, L6_HASH_KMAC_SZ);
	// Check CRC
	TEST_ASSERT_EQUAL( aCrc[0], pL2f->Crc[1]);
	TEST_ASSERT_EQUAL( aCrc[1], pL2f->Crc[0]);
}

/******************************************************************************/
// Mock

uint8_t _crypto_encrypt_cb_(
		uint8_t* p_Out,
		uint8_t* p_In,
		uint8_t u8_Sz,
		uint8_t* p_Ctr,
		uint8_t u8_KeyId,
		int cmock_num_calls
		)
{
	TEST_ASSERT_NOT_NULL(p_Out);
	TEST_ASSERT_NOT_NULL(p_In);
	TEST_ASSERT_NOT_NULL(p_Ctr);
	return CRYPTO_OK;
}
uint8_t _crypto_decrypt_cb_(
		uint8_t* p_Out,
		uint8_t* p_In,
		uint8_t u8_Sz,
		uint8_t* p_Ctr,
		uint8_t u8_KeyId,
		int cmock_num_calls
		)
{
	TEST_ASSERT_NOT_NULL(p_Out);
	TEST_ASSERT_NOT_NULL(p_In);
	TEST_ASSERT_NOT_NULL(p_Ctr);
	return CRYPTO_OK;
}

typedef enum
{
	TEST_AES_HMAC_STATUS_Match,
	TEST_AES_HMAC_STATUS_Mismatch,
	TEST_AES_HMAC_STATUS_KO
} test_aes_hmac_status_e;

#define NB_AES_HMAC_STATUS 2
test_aes_hmac_status_e eTestHMACStatus[NB_AES_HMAC_STATUS] =
{
	TEST_AES_HMAC_STATUS_Match,
	TEST_AES_HMAC_STATUS_Match
};

static int eTestHMACStatusIndex = 0;

uint8_t _crypto_aes128_cmac_xxx_cb_(
		uint8_t* p_Hash,
		uint8_t* p_Msg,
		uint8_t u8_Sz,
		uint8_t* p_Ctr,
		uint8_t u8_KeyId,
		int cmock_num_calls
		)
{
	uint8_t ret = CRYPTO_OK;
	TEST_ASSERT_NOT_NULL(p_Hash);
	TEST_ASSERT_NOT_NULL(p_Msg);
	TEST_ASSERT_NOT_NULL(p_Ctr);
	memcpy(p_Hash, aHash, L6_HASH_KENC_SZ);
	if(eTestHMACStatusIndex > NB_AES_HMAC_STATUS)
	{
		eTestHMACStatusIndex = 0;
	}
	switch(eTestHMACStatus[eTestHMACStatusIndex])
	{
		case TEST_AES_HMAC_STATUS_KO:
			ret = CRYPTO_KO;
			break;
		case TEST_AES_HMAC_STATUS_Mismatch:
			p_Hash[0] = p_Hash[1];
			break;
		case TEST_AES_HMAC_STATUS_Match:
		default :
			break;
	}
	eTestHMACStatusIndex++;
	return ret;
}



uint8_t _crypto_aes128_cmac_cb_(
		uint8_t* p_Hash,
		uint8_t* p_Msg,
		uint8_t u8_Sz,
		uint8_t* p_Ctr,
		uint8_t u8_KeyId,
		int cmock_num_calls
		)
{
	TEST_ASSERT_NOT_NULL(p_Hash);
	TEST_ASSERT_NOT_NULL(p_Msg);
	TEST_ASSERT_NOT_NULL(p_Ctr);
	memcpy(p_Hash, aHash, L6_HASH_KENC_SZ);
	return CRYPTO_OK;
}

uint8_t _crypto_aes128_cmac_mismatch_cb_(
		uint8_t* p_Hash,
		uint8_t* p_Msg,
		uint8_t u8_Sz,
		uint8_t* p_Ctr,
		uint8_t u8_KeyId,
		int cmock_num_calls
		)
{
	TEST_ASSERT_NOT_NULL(p_Hash);
	TEST_ASSERT_NOT_NULL(p_Msg);
	TEST_ASSERT_NOT_NULL(p_Ctr);
	memcpy(p_Hash, aHash, L6_HASH_KENC_SZ);
	p_Hash[1] = ~(p_Hash[1]);
	return CRYPTO_OK;
}

uint8_t _crc_compute_cb_(
		uint8_t* p_Buf,
		uint8_t u8_Sz,
		uint16_t* p_Crc,
		int cmock_num_calls
		)
{
	TEST_ASSERT_NOT_NULL(p_Buf);
	TEST_ASSERT_NOT_NULL(p_Crc);
	memcpy(p_Crc, aCrc, CRC_SZ);
	return 1;
}

uint8_t _crc_check_cb_(
		uint16_t u16_CrcA,
		uint16_t u16_CrcB,
		int cmock_num_calls
		)
{
	return 1;
}


uint8_t _rs_decode_cb_(uint8_t* p_Data, int cmock_num_calls)
{
	return 1;
}

/******************************************************************************/

TEST_SETUP(WizeCoreProto)
{
	sCtx.u8Size = 0;
	sCtx.pBuffer = aBuff;
	sCtx.sProtoConfig.filterDisL2 = 0;
	sCtx.sProtoConfig.filterDisL6 = 0;
	sCtx.sProtoConfig.u8TransLenMax = 120;
	sCtx.sProtoConfig.u8RecvLenMax = 120;
	sCtx.sProtoConfig.u8NetId = 0;

	sNetMsg.pData = aData;
	sNetMsg.u32Epoch = 0;
	sNetMsg.u16Id = 0;
	sNetMsg.u8Size = 0;
	sNetMsg.u8Type = 0;
	sNetMsg.u8Rssi = 0;
	sNetMsg.u8KeyId = 0;
	sNetMsg.u16Tstamp = 0;
	sNetMsg.Option = 0;


	sNetMsg.u8Size = sprintf(aData, "ProtoBuild");
	sNetMsg.u16Id = 0xBEEF;
	sNetMsg.u8Type = APP_DATA;
	sNetMsg.u8KeyId = 1;

	// ---
	RS_Init_Ignore();

	CRC_Compute_Stub(NULL);
	CRC_Check_Stub(NULL);

	Crypto_AES128_CMAC_Stub(NULL);
	Crypto_Encrypt_Stub(NULL);
	Crypto_Decrypt_Stub(NULL);

	RS_Decode_Stub(NULL);

	eTestHMACStatusIndex = 0;
	eTestHMACStatus[0] = TEST_AES_HMAC_STATUS_Match;
	eTestHMACStatus[1] = TEST_AES_HMAC_STATUS_Match;
}

TEST_TEAR_DOWN(WizeCoreProto)
{

}

//==============================================================================
TEST(WizeCoreProto, test_Proto_Build_NullPtr)
{
	uint8_t eRet;

	eRet = Wize_ProtoBuild(NULL, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);

	eRet = Wize_ProtoBuild(&sCtx, NULL);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);

	sCtx.pBuffer = NULL;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);

	sNetMsg.pData = NULL;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_BadSize)
{
	uint8_t eRet;
	sNetMsg.u8Size = 0;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_SZ_ERR, eRet);

	sNetMsg.u8Size = 0x20;
	sCtx.sProtoConfig.u8TransLenMax = 0x19;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_APP_MSG_SZ_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_AppTypeMismatch)
{
	uint8_t eRet;
	// Check bad frame type
	sNetMsg.u8Type = APP_DATA + 2;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_STACK_MISMATCH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_CryptoFailed)
{
	uint8_t eRet;
	// Check encryption return error
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_CIPH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_HKencFailed)
{
	uint8_t eRet;
	sNetMsg.u8KeyId = 0;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	// Check AES128 hash_kenc with keyid !=0 return error
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_HASH_ERR, eRet);

	sNetMsg.u8KeyId = 1;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	// Check AES128 hash_kenc with keyid !=0 return error
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_HASH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_HKmacFailed)
{
	uint8_t eRet;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	// Check AES128 hash_kmac  return error
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_HASH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_CrcFailed)
{
	uint8_t eRet;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	// check CRC compute error
	CRC_Compute_ExpectAnyArgsAndReturn(0);
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_CRC_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Build_L6AppGiven)
{
	uint8_t eRet;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	CRC_Compute_ExpectAnyArgsAndReturn(1);
	// payload has L6APP
	uint8_t l6app = 66;

	aData[0] = l6app;
	sNetMsg.u8Size = sprintf(&aData[1], "ProtoBuild");
	sNetMsg.u8Size++;
	sNetMsg.Option_b.App = 1;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// check L6APP
	TEST_ASSERT_EQUAL( l6app, pL6h->L6App);
}

TEST(WizeCoreProto, test_Proto_Build_L6AppDefault)
{
	uint8_t eRet;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	CRC_Compute_ExpectAnyArgsAndReturn(1);

	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// check L6APP
	TEST_ASSERT_EQUAL( sCtx.sProtoConfig.AppData, pL6h->L6App);
}

TEST(WizeCoreProto, test_Proto_Build_L6Cpt)
{
	uint8_t eRet;
	uint16_t u16Tmp;
	Crypto_Encrypt_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_OK);
	CRC_Compute_ExpectAnyArgsAndReturn(1);

	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// Check endian-ess of L6CPT
	u16Tmp = ((sNetMsg.u16Id >> 8) & 0xFF) | ( (sNetMsg.u16Id & 0xFF) << 8);
	TEST_ASSERT_EQUAL_MEMORY( (uint8_t*)(&u16Tmp), pL6h->L6Cpt, L6_CPT_SZ);
}

TEST(WizeCoreProto, test_Proto_Build_Cfield)
{
	uint8_t eRet;
	// in the next, all the 3 following return success
	Crypto_Encrypt_Stub(_crypto_encrypt_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);

	sNetMsg.u8Type = APP_DATA;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// check Cfield
	TEST_ASSERT_EQUAL( DATA, pL2h->Cfield);

	sNetMsg.u8Type = APP_DATA_PRIO;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// check Cfield
	TEST_ASSERT_EQUAL( DATA | DATA_PRIO, pL2h->Cfield);

	sNetMsg.u8Type = APP_ADMIN;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// check Cfield
	TEST_ASSERT_EQUAL( RESPONSE, pL2h->Cfield);

	sNetMsg.u8Type = APP_INSTALL;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_set_exch_buffer_ptrs_(sNetMsg.u8Size);
	// check Cfield
	TEST_ASSERT_EQUAL( INSTPING, pL2h->Cfield);
}

TEST(WizeCoreProto, test_Proto_Build_CheckLField)
{
	uint8_t eRet;
	uint8_t u8Tmp;

	// in the next, all the 3 following return success
	Crypto_Encrypt_Stub(_crypto_encrypt_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);

	// --------------------------------------------
	sCtx.sProtoConfig.u8TransLenMax = FRAME_SEND_MAX_SZ;

	sNetMsg.u8Size = 0x20;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	u8Tmp = 255 - FRAME_SEND_MAX_SZ + sNetMsg.u8Size;
	TEST_ASSERT_EQUAL( u8Tmp, aBuff[0]);

	sNetMsg.u8Size = 0xA0;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	u8Tmp = 255 - FRAME_SEND_MAX_SZ + sNetMsg.u8Size;
	TEST_ASSERT_EQUAL( u8Tmp, aBuff[0]);

	sNetMsg.u8Size = FRAME_SEND_MAX_SZ;
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	u8Tmp = 255 - FRAME_SEND_MAX_SZ + sNetMsg.u8Size;
	TEST_ASSERT_EQUAL( u8Tmp, aBuff[0]);
}

TEST(WizeCoreProto, test_Proto_Build_CheckOtherContent)
{
	uint8_t eRet;

	// in the next, all the 3 following return success
	Crypto_Encrypt_Stub(_crypto_encrypt_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);

	// --------------------------------------------
	// check that all data in out buffer are correct
	eRet = Wize_ProtoBuild(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);
	_check_exch_other_content_(sNetMsg.u8Size);
}

/******************************************************************************/
TEST(WizeCoreProto, test_Proto_Extract_NullPtr)
{
	uint8_t eRet;

	eRet = Wize_ProtoExtract(NULL, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);


	eRet = Wize_ProtoExtract(&sCtx, NULL);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);

	sCtx.pBuffer = NULL;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);

	sNetMsg.pData = NULL;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_NULL_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_Extract_BadSize)
{
	uint8_t eRet;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);

	sCtx.u8Size = 2;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_SZ_ERR, eRet);

	sCtx.u8Size = 0x20;
	sCtx.sProtoConfig.u8RecvLenMax = 0x19;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_APP_MSG_SZ_ERR, eRet);
}

/******************************************************************************/
TEST(WizeCoreProto, test_Proto_ExtractDwn_RSDecodeFailed)
{
	uint8_t eRet;
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	// Check RS_Decode failed
	RS_Decode_ExpectAnyArgsAndReturn(0);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_RS_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_CRCComputeFailed)
{
	uint8_t eRet;
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	// Check CRC compute error
	RS_Decode_ExpectAnyArgsAndReturn(1);
	CRC_Compute_ExpectAnyArgsAndReturn(0);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_CRC_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_CRCCheckErr)
{
	uint8_t eRet;
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	// Check CRC check error
	RS_Decode_ExpectAnyArgsAndReturn(1);
	CRC_Compute_ExpectAnyArgsAndReturn(1);
	CRC_Check_ExpectAnyArgsAndReturn(0);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_CRC_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_BadL2DownID)
{
	uint8_t eRet;
	// ---
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	RS_Decode_Stub(_rs_decode_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	// Check bad down ID
	_fill_dwn_buffer_ptrs_(aBuff[0]);
	pL2h_dwn->L2DownId[0] = 25;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_PASS_INF, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_BadL6DownVer)
{
	uint8_t eRet;
	// ---
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	RS_Decode_Stub(_rs_decode_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	_fill_dwn_buffer_ptrs_(aBuff[0]);
	// Check bad L6DownVer
	pL6h_dwn->L6DownVer = 66;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_DOWNLOAD_VER_WRN, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_HklogError)
{
	uint8_t eRet;
	// ---
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	RS_Decode_Stub(_rs_decode_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	_fill_dwn_buffer_ptrs_(aBuff[0]);
	// Check hklog computation error
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_HASH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_HklogMismatch)
{
	uint8_t eRet;
	// ---
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	RS_Decode_Stub(_rs_decode_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	_fill_dwn_buffer_ptrs_(aBuff[0]);
	// Check hklog mismatch
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_mismatch_cb_);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_HEAD_END_AUTH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_UncipherError)
{
	uint8_t eRet;
	// ---
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	RS_Decode_Stub(_rs_decode_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	_fill_dwn_buffer_ptrs_(aBuff[0]);
	// Check uncipher error
	Crypto_Decrypt_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_CIPH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractDwn_CheckOtherContent)
{
	uint8_t eRet;
	uint16_t u16Tmp;
	// ---
	sCtx.u8Size = 255;
	aBuff[0] = sCtx.u8Size;
	RS_Decode_Stub(_rs_decode_cb_);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	Crypto_Decrypt_Stub(_crypto_decrypt_cb_);
	_fill_dwn_buffer_ptrs_(aBuff[0]);

	// check pNetMsg : u16Id, KEY_LOG_ID, u8Size, u8Type
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, eRet);

	// Check L6DwnBNum
	u16Tmp = pL6h_dwn->L6DownBnum[1] << 8;
	u16Tmp |= pL6h_dwn->L6DownBnum[2];
	TEST_ASSERT_EQUAL(u16Tmp, sNetMsg.u16Id);
	// Check keyid
	TEST_ASSERT_EQUAL(KEY_LOG_ID, sNetMsg.u8KeyId);
	// Check message type
	TEST_ASSERT_EQUAL(APP_DOWNLOAD, sNetMsg.u8Type);
	// Check LField
	TEST_ASSERT_EQUAL(l_size, sNetMsg.u8Size);
}

/******************************************************************************/
TEST(WizeCoreProto, test_Proto_ExtractExch_AFieldMismatch)
{
	uint8_t eRet;
	_fill_exch_buffer_ptrs_(0x20);

	// Check AField mismatch
	pL2h->Afield[0] = ~(pL2h->Afield[0]);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_PASS_INF, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_MFieldMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);

	// Check MField mismatch
	pL2h->Mfield[0] = ~(pL2h->Mfield[0]);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_PASS_INF, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_CRCComputeFailed)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	// Check CRC_Compute failed
	CRC_Compute_ExpectAnyArgsAndReturn(0);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_CRC_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_CRCCheckError)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	// Check CRC_Check error
	CRC_Compute_ExpectAnyArgsAndReturn(1);
	CRC_Check_ExpectAnyArgsAndReturn(0);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_CRC_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_CiFieldMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_ExpectAnyArgsAndReturn(1);
	CRC_Check_ExpectAnyArgsAndReturn(1);
	// Check CiField mismatch
	pL2h->Cifield = ~(WIZE_PROTO_ID);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_PROTO_UNK_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_L6VersMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_ExpectAnyArgsAndReturn(1);
	CRC_Check_ExpectAnyArgsAndReturn(1);
	// Check L6Ctrl_b.VERS mismatch
	pL6h->L6Ctrl_b.VERS = (0xFF) & 0x7;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_PROTO_UNK_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_L6NetwIdMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_ExpectAnyArgsAndReturn(1);
	CRC_Check_ExpectAnyArgsAndReturn(1);
	// Check L6NetwId mismatch
	pL6h->L6NetwId = 0xFF;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_NETWID_UNK_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_CFieldMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	// Check Cfield mismatch
	pL2h->Cfield = INSTPING;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_UNK_ERR, eRet);

	pL2h->Cfield = RESPONSE;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_UNK_ERR, eRet);

	pL2h->Cfield = DATA;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_UNK_ERR, eRet);

	pL2h->Cfield = DATA_PRIO;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_FRAME_UNK_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_HKencFailed)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_KO);
	// Check HKencFailed
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_HASH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_HKencMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_mismatch_cb_);
	// Check HKencMismatch
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_HEAD_END_AUTH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_HKmacFailed)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	//Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	//Crypto_AES128_CMAC_ExpectAnyArgsAndReturn(CRYPTO_KO);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_xxx_cb_);
	// Check HKmacFailed
	eTestHMACStatus[0] = TEST_AES_HMAC_STATUS_Match;
	eTestHMACStatus[1] = TEST_AES_HMAC_STATUS_KO;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_HASH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_HKmacMismatch)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_xxx_cb_);
	// Check HKmacMismatch
	eTestHMACStatus[0] = TEST_AES_HMAC_STATUS_Match;
	eTestHMACStatus[1] = TEST_AES_HMAC_STATUS_Mismatch;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_GATEWAY_AUTH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_UnciphFailed)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);

	// Check UnciphFailed
	Crypto_Decrypt_ExpectAnyArgsAndReturn(CRYPTO_KO);
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_INTERNAL_CIPH_ERR, eRet);
}

TEST(WizeCoreProto, test_Proto_ExtractExch_UnknownKid)
{
	uint8_t eRet;

	_fill_exch_buffer_ptrs_(0x20);
	CRC_Compute_Stub(_crc_compute_cb_);
	CRC_Check_Stub(_crc_check_cb_);
	Crypto_AES128_CMAC_Stub(_crypto_aes128_cmac_cb_);
	Crypto_Decrypt_Stub(_crypto_decrypt_cb_);

	// Check UnknownKid PROTO_KEYID_UNK_ERR
	// not possible case !!!!
	pL6h->L6Ctrl_b.KEYSEL = (0x1F) & 0xF;
	eRet = Wize_ProtoExtract(&sCtx, &sNetMsg);
	TEST_ASSERT_EQUAL(PROTO_KEYID_UNK_ERR, eRet);
}

/******************************************************************************/
TEST(WizeCoreProto, test_Proto_StatRxUpdate_Success)
{
	sCtx.u8Size = 10;
	sCtx.sProtoStats.u32RxNbBytes = 5;
	Wize_ProtoStats_RxUpdate(&sCtx, 4, 155);
	TEST_ASSERT_EQUAL(15, sCtx.sProtoStats.u32RxNbBytes);
}

TEST(WizeCoreProto, test_Proto_StatsTxUpdate_Success)
{
	sCtx.u8Size = 10;
	sCtx.sProtoStats.u32TxNbBytes = 5;
	Wize_ProtoStats_TxUpdate(&sCtx, 5, 128);
	TEST_ASSERT_EQUAL(15, sCtx.sProtoStats.u32TxNbBytes);
}

TEST(WizeCoreProto, test_Proto_StatsRxClear_Success)
{
	uint8_t *p = (uint8_t*)( &(sCtx.sProtoStats) );
	memset(p, 0, sizeof(struct proto_stats_s));
	sCtx.sProtoStats.u8RxRssiMax = 0x0A;
	sCtx.sProtoStats.u8RxRssiAvg = 0xCB;
	sCtx.sProtoStats.u32RxNbFrmOK = 0x1F02;
	sCtx.sProtoStats.sFrmErrStats.u32GatewayAuthErr = 0xEF;

	Wize_ProtoStats_RxClear(&sCtx);
	for (int i =0; i < sizeof(struct proto_stats_s); i++ )
	{
		TEST_ASSERT_EQUAL(0, p[i]);
	}
}

TEST(WizeCoreProto, test_Proto_StatsTxClear_Success)
{
	uint8_t *p = (uint8_t*)( &(sCtx.sProtoStats) );
	memset(p, 0, sizeof(struct proto_stats_s));
	sCtx.sProtoStats.u8TxNoiseMin = 0x0A;
	sCtx.sProtoStats.u8TxReserved = 0xCB;
	sCtx.sProtoStats.u32TxNbBytes = 0x1F02;

	Wize_ProtoStats_TxClear(&sCtx);
	for (int i =0; i < sizeof(struct proto_stats_s); i++ )
	{
		TEST_ASSERT_EQUAL(0, p[i]);
	}
}
