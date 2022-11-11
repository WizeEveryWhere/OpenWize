#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <machine/endian.h>

TEST_GROUP(WizeCoreAppInt_adm);
#include "adm_internal.h"
#include "parameters_lan_ids.h"
#include "phy_layer.h"

// Mock Samples
#include "mock_crypto.h"
#include "mock_parameters.h"

int32_t local_update_pending;
/******************************************************************************/
struct adm_config_s sAdmConfig;

int32_t AdmInt_AnnIsLocalUpdate(void)
{
	return local_update_pending;
}

/******************************************************************************/
#define HW_VER 0x1234
#define FW_VER 0x5678
#define MF 0x9ABC
#define NB_BLK 814

#define BUFFER_SZ_MAX 229
uint8_t gaDataReq[BUFFER_SZ_MAX] = {0xFF, 53, 25, 10, 18};
net_msg_t gsReqMsg = { .pData = gaDataReq, .u8Size = 5, .u8KeyId = 0};

uint8_t gaDataRsp[BUFFER_SZ_MAX];
net_msg_t gsRspMsg = { .pData = gaDataRsp, };

admin_rsp_err_t *gpErrRsp = ((admin_rsp_err_t*)gaDataRsp);
admin_rsp_t *gpRsp = ((admin_rsp_t*)gaDataRsp);

uint8_t trans_len_max = 0;

static uint8_t __Param_Access_stub__(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir, int cmock_num_calls)
{
	uint8_t ret = 1;
	if(!u8_Dir)
	{
		switch(u8_Id)
		{
			case L7TRANSMIT_LENGTH_MAX:
				*p_Data = trans_len_max;
				break;
			case VERS_HW_TRX:
				*(uint16_t*)p_Data = __htons((uint16_t)HW_VER);
				break;
			case VERS_FW_TRX:
				*(uint16_t*)p_Data = __htons((uint16_t)FW_VER);
				break;
			default:

				break;
		}
	}
	return ret;
}

/******************************************************************************/
static void _fill_ann_(void);
static void _fill_fwinfo_(admin_ann_fw_info_t *pFwInfo);

static void _fill_ann_(void)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	pReq->L7CommandId = ADM_ANNDOWNLOAD;
	*(uint16_t*)pReq->L7SwVersionIni = __htons( (uint16_t)FW_VER );
	*(uint16_t*)pReq->L7SwVersionTarget = __htons( (uint16_t)(FW_VER +1) );
	*(uint16_t*)pReq->L7MField = sAdmConfig.MField;
	*(uint16_t*)pReq->L7DcHwId = __htons((uint16_t)HW_VER);
	*(uint16_t*)pReq->L7BlocksCount = __htons( (uint16_t)(sAdmConfig.u32DwnBlkNbMax) );
	pReq->L7ChannelId = 120;
	pReq->L7ModulationId = PHY_WM2400;
	time_t t;
	time(&t);
	/*
	 * |<--------------------   DayProg window   ------------------------->|
	 * |<--   u32DwnDayProgWinMin   -->|<----   u32DwnDayProgWinMax   ---->|
	 *                  currentEpoch __^
	 *
	 *     _________________________
	 * ---|_________________________|--------------------------------------|
	 *    |<-- u32MntWinDuration -->|                                      |
	 *    ^
	 * 00:00 UTC
	 *
	*/
	t -= EPOCH_UNIX_TO_OURS;
	// get next MNT window
	t = ( (t / 86400) + 1) *86400;
	//t += sAdmConfig.u32DwnDayProgWinMin;
	//t += (sAdmConfig.u32DwnDayProgWinMax - sAdmConfig.u32DwnDayProgWinMin)/2;
	*(uint32_t*)pReq->L7DaysProg = __htonl(t);

	pReq->L7DayRepeat = 3;
	pReq->L7DeltaSec = DWN_DELTA_SEC_MIN_WM2400;

	// -----
	gsReqMsg.u8KeyId = KEY_CHG_ID;
	gsReqMsg.u8Size = sizeof(admin_cmd_anndownload_t);

	((admin_rsp_t*)gaDataRsp)->L7ErrorCode = ADM_NONE;
}

static void _fill_fwinfo_(admin_ann_fw_info_t *pFwInfo)
{
	pFwInfo->u16DcHwId = (uint16_t)HW_VER;
	pFwInfo->u16SwVerIni = (uint16_t)FW_VER;
	pFwInfo->u16SwVerTgt = (uint16_t)(FW_VER + 1);
	pFwInfo->u16BlkCnt = (uint16_t)NB_BLK;
	pFwInfo->u32HashSW = 0;
}
/******************************************************************************/

TEST_SETUP(WizeCoreAppInt_adm)
{
	Crypto_WriteKey_IgnoreAndReturn(CRYPTO_OK);

	//Param_Access_IgnoreAndReturn(1);
	Param_Access_Stub(__Param_Access_stub__);

	Param_IsValidId_IgnoreAndReturn(1);
	Param_GetSize_IgnoreAndReturn(1);
	Param_RemoteAccess_IgnoreAndReturn(1);
	Param_GetRemAccess_IgnoreAndReturn(RW);
	Param_CheckConformity_IgnoreAndReturn(1);

	trans_len_max = 120;
	local_update_pending = -1;
	sAdmConfig.MField = (uint16_t)MF;
	sAdmConfig.u32MntWinDuration = MNT_WIN_DURATION;
}

TEST_TEAR_DOWN(WizeCoreAppInt_adm)
{

}

/******************************************************************************/
// ADM unknown command

TEST(WizeCoreAppInt_adm, test_AdmInt_UnknownCmd)
{
	admin_rsp_cmderr_t *pErr = ((admin_rsp_cmderr_t*)gaDataRsp);
	AdmInt_Unknown(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_UNK_CMD, pErr->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_cmderr_t), gsRspMsg.u8Size, "Rsp size");
}

/******************************************************************************/
// ADM_READ_PARAM

TEST(WizeCoreAppInt_adm, test_AdmInt_ReadInvalidParam)
{
	// One parameter is invalid
	Param_IsValidId_IgnoreAndReturn(0);
	AdmInt_ReadParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(READ_UNK_PARAM, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_ReadLenExceed)
{
	uint8_t expected_sz;
	// The maximum admitted size is exceed
	trans_len_max = 20;
	// let parameter size equal to trans_len_max
	Param_GetSize_IgnoreAndReturn(trans_len_max);
	// expected size is : sizeof(admin_rsp_t) + sizeof(paramid) + len(param)
	expected_sz = sizeof(admin_rsp_t) + 1 + trans_len_max;

	AdmInt_ReadParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(READ_LENGTH_EXCEED, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected_sz,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_ReadAcessDeniedWO)
{
	// Read access is denied ==> WO
	Param_RemoteAccess_IgnoreAndReturn(0);
	Param_GetRemAccess_IgnoreAndReturn(WO);
	AdmInt_ReadParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(READ_ACCES_DENIED, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_ReadAcessDeniedNA)
{
	// Read access is denied ==> NA
	Param_RemoteAccess_IgnoreAndReturn(0);
	Param_GetRemAccess_IgnoreAndReturn(NA);
	AdmInt_ReadParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(READ_UNK_PARAM, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}
/******************************************************************************/
// ADM_WRITE_PARAM

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteInvalidParam)
{
	// One parameter is invalid
	Param_IsValidId_IgnoreAndReturn(0);
	AdmInt_WriteParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WRITE_UNK_PARAM, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteAcessDeniedRO)
{
	// Write access is denied ==> RO
	Param_RemoteAccess_IgnoreAndReturn(0);
	Param_GetRemAccess_IgnoreAndReturn(RO);
	AdmInt_WriteParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WRITE_ACCES_DENIED, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteAcessDeniedNA)
{
	// Write access is denied ==> NA
	Param_RemoteAccess_IgnoreAndReturn(0);
	Param_GetRemAccess_IgnoreAndReturn(NA);
	AdmInt_WriteParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WRITE_UNK_PARAM, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteNotConform)
{
	gaDataReq[0] = ADM_WRITE_PARAM;
	// The write value is not conform
	Param_CheckConformity_IgnoreAndReturn(0);
	AdmInt_WriteParam(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WRITE_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[1],  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

/******************************************************************************/
// ADM_WRITE_KEY

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteKeyBadFrmLen)
{
	// Frame length doesn't match
	gsReqMsg.u8Size = 5;
	AdmInt_WriteKey(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(KEY_INCORRECT_FRM_LEN, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Size,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteKeyWrongCiphKey)
{
	gsReqMsg.u8Size = sizeof(admin_cmd_writekey_t);
	// key change not use
	gsReqMsg.u8KeyId = 1;
	AdmInt_WriteKey(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(KEY_KCHG_NOT_USED, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8KeyId,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteKeyBadId)
{
	admin_cmd_writekey_t *pCmd = (admin_cmd_writekey_t*)(gaDataReq);
	gsReqMsg.u8Size = sizeof(admin_cmd_writekey_t);
	gsReqMsg.u8KeyId = KEY_CHG_ID;

	sAdmConfig.WriteKeyBypassId = 0;
	// Key id is forbidden
	pCmd->L7KeyId = 0x00;
	AdmInt_WriteKey(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(KEY_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");

	AdmInt_SetupDefaultConfig();
}

TEST(WizeCoreAppInt_adm, test_AdmInt_WriteKeyBadIdByPassKeyId)
{
	admin_cmd_writekey_t *pCmd = (admin_cmd_writekey_t*)(gaDataReq);
	gsReqMsg.u8Size = sizeof(admin_cmd_writekey_t);
	gsReqMsg.u8KeyId = KEY_CHG_ID;

	sAdmConfig.WriteKeyBypassId = 1;
	// Key id is forbidden
	pCmd->L7KeyId = 0x00;
	AdmInt_WriteKey(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(KEY_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");

	pCmd->L7KeyId = KEY_MAX_NB;
	AdmInt_WriteKey(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(KEY_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");

	AdmInt_SetupDefaultConfig();
}
/******************************************************************************/
// ADM_EXECINSTPING
// No error case

/******************************************************************************/
// ADM_ANNDOWNLOAD

//-------------------------------------------------
// Check on session

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadFrmLen)
{
	_fill_ann_();
	// Frame length doesn't match
	gsReqMsg.u8Size = 5;
	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_INCORRECT_FRM_LEN, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Size,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnLocalUpadtePending)
{
	_fill_ann_();
	// A local update is already in progress
	local_update_pending = 25;
	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_UPD_IN_PROGRESS, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)local_update_pending,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnMFieldMismatch)
{
	_fill_ann_();
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	// MField mismatch
	*(uint16_t*)pReq->L7MField = ~(sAdmConfig.MField);

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_FIELD_ID_L7MField,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadKeyId)
{
	_fill_ann_();

	// Not keyChg
	gsReqMsg.u8KeyId = (uint8_t)(~(KEY_CHG_ID));

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_KCHG_NOT_USED, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8KeyId,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadChannel)
{
	_fill_ann_();
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);

	// Bad channel
	pReq->L7ChannelId = 160;

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_FIELD_ID_L7ChannelId,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadModulation)
{
	_fill_ann_();
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);

	// Bad modulation
	pReq->L7ModulationId = PHY_WM6400;

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_FIELD_ID_L7ModulationId,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnDayRepeatOutOfScope)
{
	_fill_ann_();
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);

	// Bad Day repeat
	pReq->L7DayRepeat = 0;

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_FIELD_ID_L7DayRepeat,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnDeltaSecOutOfScope)
{
	_fill_ann_();
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);

	// DeltaSec out of scope
	pReq->L7DeltaSec = 0;

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_ILLEGAL_VALUE, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_FIELD_ID_L7DeltaSec,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBcastDayOutOfScopeLower)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	int8_t expected;
	time_t currentEpoch, t;

	_fill_ann_();
	// Day prog out of scope : lower than u32DwnDayProgWinMin
	time(&currentEpoch);
	currentEpoch -= EPOCH_UNIX_TO_OURS;
	t = currentEpoch;
	if ( t > (sAdmConfig.u32DwnDayProgWinMin + 1))
	{
		t -= (sAdmConfig.u32DwnDayProgWinMin + 1);
	}
	else
	{
		t = 0;
	}
	expected = (int8_t)(currentEpoch - t);
	*(uint32_t*)pReq->L7DaysProg = __htonl(t);
	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_INCORRECT_BCAST_START_DAY, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(expected,  (int8_t)gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBcastDayOutOfScopeUpper)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	int8_t expected;
	time_t currentEpoch, t;

	_fill_ann_();
	// Day prog out of scope : greater than u32DwnDayProgWinMax
	time(&currentEpoch);
	currentEpoch -= EPOCH_UNIX_TO_OURS;
	t = currentEpoch;
	t += (sAdmConfig.u32DwnDayProgWinMax + 1);
	expected = (int8_t)(currentEpoch - t);
	*(uint32_t*)pReq->L7DaysProg = __htonl(t);
	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_INCORRECT_BCAST_START_DAY, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(expected,  (int8_t)gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowDurationOutOfScope)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	_fill_ann_();

	// Bcast duration is too large
	sAdmConfig.u32MntWinDuration = 0;
	pReq->L7DeltaSec = 255;

	uint32_t temp = sAdmConfig.u32DwnBlkDurMod + pReq->L7DeltaSec*1000;
	uint16_t blkNb;
	blkNb = ((86400 + pReq->L7DeltaSec) *1000) / (temp);
	blkNb += 1;
	*(uint16_t*)pReq->L7BlocksCount = __htons(blkNb);

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_DIFF_TIME_OUT_OF_WINDOWS, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xFF,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowOutOfScopeMntBeg)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	time_t currentEpoch, t;

	// Bcast begin is out of maintenance windows
	_fill_ann_();
	time(&currentEpoch);
	currentEpoch -= EPOCH_UNIX_TO_OURS;
	t = ( (currentEpoch / 86400) + 1) *86400 + sAdmConfig.u32MntWinDuration + 1;
	*(uint32_t*)pReq->L7DaysProg = __htonl(t);

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_DIFF_TIME_OUT_OF_WINDOWS, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowOutOfScopeMntEnd)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	time_t currentEpoch, t;

	uint16_t blkNb;
	uint32_t temp = sAdmConfig.u32DwnBlkDurMod + pReq->L7DeltaSec*1000;

	// Bcast begin + duration is out of maintenance windows, but duration can fit MNT window
	_fill_ann_();
	time(&currentEpoch);
	currentEpoch -= EPOCH_UNIX_TO_OURS;
#define OFSSET (uint32_t)10

	t = ( (currentEpoch / 86400) + 1) *86400 + OFSSET*temp;
	*(uint32_t*)pReq->L7DaysProg = __htonl(t);

	blkNb = (sAdmConfig.u32MntWinDuration *1000) / (temp);
	*(uint16_t*)pReq->L7BlocksCount = __htons(blkNb);

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_DIFF_TIME_OUT_OF_WINDOWS, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowOutOfScopeMnt)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(gaDataReq);
	time_t currentEpoch, t;

	// -----
	uint16_t blkNb;
	uint32_t temp = sAdmConfig.u32DwnBlkDurMod + pReq->L7DeltaSec*1000;

	// Bcast duration is larger than maintenance window
	_fill_ann_();
	blkNb = ((sAdmConfig.u32MntWinDuration + pReq->L7DeltaSec) *1000) / (temp);
	blkNb += 10;
	*(uint16_t*)pReq->L7BlocksCount = __htons(blkNb);

	AdmInt_Anndownload(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_DIFF_TIME_OUT_OF_WINDOWS, gpErrRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xFF,  gpErrRsp->L7ErrorParam, "Error param");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(sizeof(admin_rsp_err_t), gsRspMsg.u8Size, "Rsp size");
}

//-------------------------------------------------
// Check on internal FW

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadNbOfBlock)
{
	uint8_t ret;
	uint8_t u8ErrorParam;
	admin_ann_fw_info_t sFwInfo;
	_fill_fwinfo_(&sFwInfo);

	// Nb of block is too large
	sFwInfo.u16BlkCnt += 1 ;
	ret = AdmInt_AnnCheckIntFW(&sFwInfo, &u8ErrorParam);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_ILLEGAL_VALUE, ret, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_FIELD_ID_L7BlocksCount,  u8ErrorParam, "Error param");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadTargetHW)
{
	uint8_t ret;
	uint8_t u8ErrorParam;
	admin_ann_fw_info_t sFwInfo;
	_fill_fwinfo_(&sFwInfo);

	uint8_t expected = (uint8_t)((uint16_t)HW_VER & 0xFF);

	// HW id mismatch
	sFwInfo.u16DcHwId += 1 ;
	ret = AdmInt_AnnCheckIntFW(&sFwInfo, &u8ErrorParam);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_INCORRECT_HW_VER, ret, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected,  u8ErrorParam, "Error param");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadInitFW)
{
	uint8_t ret;
	uint8_t u8ErrorParam;
	admin_ann_fw_info_t sFwInfo;
	_fill_fwinfo_(&sFwInfo);

	uint8_t expected = (uint8_t)((uint16_t)FW_VER & 0xFF);

	// SW ini ver mismatch
	sFwInfo.u16SwVerIni += 1 ;
	ret = AdmInt_AnnCheckIntFW(&sFwInfo, &u8ErrorParam);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_INCORRECT_INI_SW_VER, ret, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected,  u8ErrorParam, "Error param");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadTargetFWLower)
{
	uint8_t ret;
	uint8_t u8ErrorParam;
	admin_ann_fw_info_t sFwInfo;
	_fill_fwinfo_(&sFwInfo);

	uint8_t expected = (uint8_t)((uint16_t)FW_VER & 0xFF);

	// SW target ver lower
	sFwInfo.u16SwVerTgt = (uint16_t)FW_VER - 1 ;
	ret = AdmInt_AnnCheckIntFW(&sFwInfo, &u8ErrorParam);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_TGT_SW_VER, ret, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected,  u8ErrorParam, "Error param");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_AnnBadTargetFWEqual)
{
	uint8_t ret;
	uint8_t u8ErrorParam;
	admin_ann_fw_info_t sFwInfo;
	_fill_fwinfo_(&sFwInfo);

	uint8_t expected = (uint8_t)((uint16_t)FW_VER & 0xFF);

	// SW target ver already diownloaded
	sFwInfo.u16SwVerTgt = (uint16_t)FW_VER ;
	ret = AdmInt_AnnCheckIntFW(&sFwInfo, &u8ErrorParam);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ANN_TGT_VER_DWL, ret, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected,  u8ErrorParam, "Error param");
}

/******************************************************************************/
// AdmInt_PreCmd

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdUNKOWN)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	gsReqMsg.u8Rssi = 25;

	// ADM unknown command
	gaDataReq[0] = 0xF9;
	gsRspMsg.u16Id = gsReqMsg.u16Id - 1;
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_READY,  ret, "Return value");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(gsReqMsg.u16Id, gsRspMsg.u16Id, "Msg id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[0], gpRsp->L7ResponseId, "Rsp Id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_UNK_CMD,  gpRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)sizeof(admin_rsp_cmderr_t),  gsRspMsg.u8Size, "Size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdREAD_PARAM)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	gsReqMsg.u8Rssi = 25;

	// ADM_READ_PARAM
	gaDataReq[0] = ADM_READ_PARAM;
	gsRspMsg.u16Id = gsReqMsg.u16Id - 1;
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_READY,  ret, "Return value");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(gsReqMsg.u16Id, gsRspMsg.u16Id, "Msg id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[0],  gpRsp->L7ResponseId, "Rsp Id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_NONE,  gpRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Rssi, gpRsp->L7Rssi, "RSSI");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE((uint16_t)FW_VER , __ntohs( *(uint16_t*)(gpRsp->L7SwVersion) ), "SW version");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(sizeof(admin_rsp_t) + (gsReqMsg.u8Size -1)*2 ),  gsRspMsg.u8Size, "Size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdWRITE_PARAM)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	gsReqMsg.u8Rssi = 25;

	// ADM_WRITE_PARAM
	gaDataReq[0] = ADM_WRITE_PARAM;
	gsRspMsg.u16Id = gsReqMsg.u16Id - 1;
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_READY,  ret, "Return value");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(gsReqMsg.u16Id, gsRspMsg.u16Id, "Msg id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[0],  gpRsp->L7ResponseId, "Rsp Id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_NONE,  gpRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Rssi, gpRsp->L7Rssi, "RSSI");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE((uint16_t)FW_VER , __ntohs( *(uint16_t*)(gpRsp->L7SwVersion) ), "SW version");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(sizeof(admin_rsp_t)),  gsRspMsg.u8Size, "Size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdEXECINSTPING)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	gsReqMsg.u8Rssi = 25;

	// ADM_EXECINSTPING
	gaDataReq[0] = ADM_EXECINSTPING;
	gsRspMsg.u16Id = gsReqMsg.u16Id - 1;
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_NOT_READY,  ret, "Return value");
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_ALREADY_DONE,  ret, "2nd Return value");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(gsReqMsg.u16Id, gsRspMsg.u16Id, "Msg id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[0],  gpRsp->L7ResponseId, "Rsp Id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_NONE,  gpRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Rssi, gpRsp->L7Rssi, "RSSI");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE((uint16_t)FW_VER , __ntohs( *(uint16_t*)(gpRsp->L7SwVersion) ), "SW version");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(sizeof(admin_rsp_execinstping_t)),  gsRspMsg.u8Size, "Size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdWRITE_KEY)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	gsReqMsg.u8Rssi = 25;

	// ADM_WRITE_KEY
	gaDataReq[0] = ADM_WRITE_KEY;
	gsRspMsg.u16Id = gsReqMsg.u16Id - 1;
	gsReqMsg.u8Size = sizeof(admin_cmd_writekey_t);
	gsReqMsg.u8KeyId = KEY_CHG_ID;
	((admin_cmd_writekey_t*)(gaDataReq))->L7KeyId = 0x01;
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_READY,  ret, "Return value");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(gsReqMsg.u16Id, gsRspMsg.u16Id, "Msg id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[0],  gpRsp->L7ResponseId, "Rsp Id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_NONE,  gpRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Rssi, gpRsp->L7Rssi, "RSSI");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE((uint16_t)FW_VER , __ntohs( *(uint16_t*)(gpRsp->L7SwVersion) ), "SW version");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(sizeof(admin_rsp_t)),  gsRspMsg.u8Size, "Size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdANNDOWNLOAD)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	gsReqMsg.u8Rssi = 25;

	// ADM_ANNDOWNLOAD
	gaDataReq[0] = ADM_ANNDOWNLOAD;
	gsRspMsg.u16Id = gsReqMsg.u16Id - 1;
	_fill_ann_();
	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_NOT_READY,  ret, "Return value");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(gsReqMsg.u16Id, gsRspMsg.u16Id, "Msg id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gaDataReq[0],  gpRsp->L7ResponseId, "Rsp Id");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(ADM_NONE,  gpRsp->L7ErrorCode, "Error code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(gsReqMsg.u8Rssi, gpRsp->L7Rssi, "RSSI");
	TEST_ASSERT_EQUAL_UINT16_MESSAGE((uint16_t)FW_VER , __ntohs( *(uint16_t*)(gpRsp->L7SwVersion) ), "SW version");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(sizeof(admin_rsp_t)),  gsRspMsg.u8Size, "Size");
}

TEST(WizeCoreAppInt_adm, test_AdmInt_PreCmdAlreadyTreat)
{
	uint8_t ret;

	gsReqMsg.u16Id = 5;
	// Command already received
	gsRspMsg.u16Id = gsReqMsg.u16Id;

	ret = AdmInt_PreCmd(&gsReqMsg, &gsRspMsg);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(RSP_ALREADY_DONE,  ret, "Return value");
}

/******************************************************************************/
// AdmInt_PostCmd


TEST(WizeCoreAppInt_adm, test_AdmInt_PostCmd)
{

}

