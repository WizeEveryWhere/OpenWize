#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreNet);
#include "net_api_private.h"

#include "mock_proto_api.h"

/******************************************************************************/

#define WIZE_REV 1.2
uint8_t L6KmacIndex = 18;
uint8_t CIPH_CURRENT_KEY = 1;

const uint8_t a_DeviceManufacturerID[2] = {0xF7, 0x25 };
const uint8_t a_DeviceInfo[6] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

//#include "key_priv.h"
//key_s _a_Key_[KEY_MAX_NB];

//#include "golden_admin.h"
uint8_t frm[255];

/******************************************************************************/
int32_t i32PhyRetCode = PHY_STATUS_OK;
uint8_t u8PhyExpectSize = 0;

uint8_t aData[256];
net_msg_t sNetMsg;

static struct phydev_s sPhydev;
static netdev_t sNetDev;
static wize_net_t sWizeCtx;

#define SETUP_CHECKED 0x01
#define INIT_CHECKED 0x02
#define UNINIT_CHECKED 0x04

uint8_t _already_check_ = 0;

/******************************************************************************/
/******************************************************************************/
// proto
const char * const proto_err_msg[] =
{
	[PROTO_PROTO_UNK_ERR] = "QED, quod erat demonstrandum"
};


uint8_t __Wize_ProtoBuild__stub__(struct proto_ctx_s* pCtx, net_msg_t* pNetMsg, int cmock_num_calls)
{
	pCtx->u8Size = 10;
	return PROTO_SUCCESS;
}
uint8_t __Wize_ProtoExtract__stub__(struct proto_ctx_s* pCtx, net_msg_t* pNetMsg, int cmock_num_calls)
{
	pCtx->u8Size = 10;
	return PROTO_SUCCESS;
}

/******************************************************************************/
/******************************************************************************/
// PHY
static int32_t Init(phydev_t *pPhydev)
{
	return i32PhyRetCode;
}

static int32_t UnInit(phydev_t *pPhydev)
{
	return i32PhyRetCode;
}

static int32_t Tx(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation)
{
	return i32PhyRetCode;
}

static int32_t Rx(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation)
{
	return i32PhyRetCode;
}

static int32_t Noise(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation)
{
	return i32PhyRetCode;
}

static int32_t SetSend(phydev_t *pPhydev, uint8_t *pBuf, uint8_t u8Len)
{
	return i32PhyRetCode;
}

static int32_t GetRecv(phydev_t *pPhydev, uint8_t *pBuf, uint8_t *u8Len)
{
//#define FRM (L2_FRM_COMMAND_READPARAMETERS)
#define FRM (frm)
	uint8_t L2sz = sizeof(FRM) -1;
	*u8Len = L2sz;
	memcpy(pBuf, &(FRM[1]), L2sz);
	return i32PhyRetCode;
}

static int32_t Ioctl(phydev_t *pPhydev, uint32_t eCtl, uint32_t args)
{
	return i32PhyRetCode;
}

static void _TxDone(void)
{
	sPhydev.pfEvtCb(sPhydev.pCbParam, PHYDEV_EVT_TX_COMPLETE);
}

static void _RxDone(void)
{
	sPhydev.pfEvtCb(sPhydev.pCbParam, PHYDEV_EVT_RX_COMPLETE);
}

const static phy_if_t sPhyIf =
{
	.pfInit = Init,
	.pfUnInit = UnInit,
	.pfTx = Tx,
	.pfRx = Rx,
	.pfNoise = Noise,
	.pfSetSend = SetSend,
	.pfGetRecv = GetRecv,
	.pfIoctl = Ioctl
};
/******************************************************************************/
static void pfcbEvent(uint32_t evt)
{

}

/******************************************************************************/
/******************************************************************************/
TEST_SETUP(WizeCoreNet)
{
	int32_t i32Ret;
	i32PhyRetCode = PHY_STATUS_OK;
	u8PhyExpectSize = 0;

	sPhydev.pIf = &sPhyIf;

	if (_already_check_ & (SETUP_CHECKED | INIT_CHECKED))
	{
		i32Ret = WizeNet_Setup(&sNetDev, &sWizeCtx, &sPhydev);
		i32Ret = WizeNet_Init(&sNetDev, pfcbEvent);
	}
/*
	sWizeCtx.sMediumCfg.eRxChannel = PHY_CH100;
	sWizeCtx.sMediumCfg.eRxModulation = PHY_WM2400;
	sWizeCtx.sMediumCfg.eTxChannel = PHY_CH100;
	sWizeCtx.sMediumCfg.eTxModulation = PHY_WM2400;
	sWizeCtx.sMediumCfg.eTxPower = PHY_PMAX_minus_0db;
	sWizeCtx.sMediumCfg.i16TxFreqOffset = 0;
*/
	sWizeCtx.sProtoCtx.sProtoConfig.AppAdm = L6APP_ADM;
	sWizeCtx.sProtoCtx.sProtoConfig.AppInst = L6APP_INST;
	sWizeCtx.sProtoCtx.sProtoConfig.AppData = 0xFE;
	sWizeCtx.sProtoCtx.sProtoConfig.u8NetId = L6KmacIndex;
	sWizeCtx.sProtoCtx.sProtoConfig.filterDisL2 = 0;
	sWizeCtx.sProtoCtx.sProtoConfig.filterDisL6 = 0;
	sWizeCtx.sProtoCtx.sProtoConfig.u8TransLenMax = 128;
	sWizeCtx.sProtoCtx.sProtoConfig.u8RecvLenMax = 128;

	memcpy(sWizeCtx.sProtoCtx.aDeviceManufID, a_DeviceManufacturerID, sizeof(a_DeviceManufacturerID));
	memcpy(sWizeCtx.sProtoCtx.aDeviceAddr, a_DeviceInfo, sizeof(a_DeviceInfo));


	Wize_ProtoBuild_IgnoreAndReturn(PROTO_SUCCESS);
	Wize_ProtoExtract_IgnoreAndReturn(PROTO_SUCCESS);
	Wize_ProtoStats_RxUpdate_Ignore();
	Wize_ProtoStats_TxUpdate_Ignore();
	Wize_ProtoStats_RxClear_Ignore();
	Wize_ProtoStats_TxClear_Ignore();
	/*
	Wize_ProtoBuild_Stub(__Wize_ProtoBuild__stub__);
	Wize_ProtoExtract_Stub(__Wize_ProtoExtract__stub__);

	Wize_ProtoBuild_ExpectAnyArgsAndReturn(PROTO_SUCCESS);
	Wize_ProtoExtract_ExpectAnyArgsAndReturn(PROTO_SUCCESS);
	Wize_ProtoStats_RxUpdate_ExpectAnyArgs();
	Wize_ProtoStats_TxUpdate_ExpectAnyArgs();
	Wize_ProtoStats_RxClear_ExpectAnyArgs();
	Wize_ProtoStats_TxClear_ExpectAnyArgs();
	*/
}

TEST_TEAR_DOWN(WizeCoreNet)
{
	int32_t i32Ret;
	if (_already_check_ & (UNINIT_CHECKED))
	{
		i32Ret = WizeNet_Uninit(&sNetDev);
	}
}

/******************************************************************************/
static void _show_err_msg_(int32_t expected, int32_t actual)
{
	if (sNetDev.eErrType == NETDEV_ERROR_PROTO)
	{
		TEST_ASSERT_EQUAL_MESSAGE(expected, actual, wize_err_msg[sWizeCtx.u8ProtoErr]);
	}
	else if (sNetDev.eErrType == NETDEV_ERROR_PHY)
	{
		TEST_ASSERT_EQUAL_MESSAGE(expected, actual, "PHY error");
	}
	else if (sNetDev.eErrType == NETDEV_ERROR_NET)
	{
		TEST_ASSERT_EQUAL_MESSAGE(expected, actual, "Net error");
	}
}

static void _clean_state_(void)
{
	sNetDev.eErrType = NETDEV_ERROR_NONE;
	sNetDev.eState   = NETDEV_STATE_IDLE;
	i32PhyRetCode = PHY_STATUS_OK;
}
/******************************************************************************/

TEST(WizeCoreNet, test_NetApi_Setup)
{
	int32_t i32Ret;

	i32Ret = WizeNet_Setup(&sNetDev, &sWizeCtx, &sPhydev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	TEST_ASSERT_EQUAL((uint32_t)(&sWizeCtx), (uint32_t)(sNetDev.pCtx));
	TEST_ASSERT_EQUAL((uint32_t)(&sPhydev), (uint32_t)(sNetDev.pPhydev));
	TEST_ASSERT_EQUAL(NETDEV_STATE_UNKWON, sNetDev.eState);

	i32Ret = WizeNet_Setup(NULL, NULL, NULL);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);

	i32Ret = WizeNet_Setup(&sNetDev, NULL, NULL);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	_already_check_ |= SETUP_CHECKED;
}

TEST(WizeCoreNet, test_NetApi_Init)
{
	int32_t i32Ret;
	i32Ret = WizeNet_Setup(&sNetDev, &sWizeCtx, &sPhydev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);

	i32Ret = WizeNet_Init(&sNetDev, pfcbEvent);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);

	i32Ret = WizeNet_Init(NULL, pfcbEvent);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);

	i32Ret = WizeNet_Init(&sNetDev, NULL);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);

	sNetDev.pPhydev = NULL;
	i32Ret = WizeNet_Init(&sNetDev, pfcbEvent);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	sNetDev.pPhydev = &sPhydev;

	_already_check_ |= INIT_CHECKED;
}

TEST(WizeCoreNet, test_NetApi_Uinit)
{
	int32_t i32Ret;
	i32Ret = WizeNet_Setup(&sNetDev, &sWizeCtx, &sPhydev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);

	i32Ret = WizeNet_Uninit(&sNetDev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_STATE_UNKWON, sNetDev.eState);

	i32Ret = WizeNet_Uninit(NULL);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);

	i32PhyRetCode = PHY_STATUS_ERROR;
	i32Ret = WizeNet_Uninit(&sNetDev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_STATE_UNKWON, sNetDev.eState);
	i32PhyRetCode = PHY_STATUS_OK;

	sNetDev.pPhydev = NULL;
	i32Ret = WizeNet_Uninit(&sNetDev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	sNetDev.pPhydev = &sPhydev;

	_already_check_ |= UNINIT_CHECKED;
}

TEST(WizeCoreNet, test_NetApi_Ioctl)
{
	int32_t i32Ret;
	uint32_t eCtl;
	uint32_t args;
	args = (uint32_t)(aData);

	// Check stats
	uint16_t sz = sizeof(struct proto_stats_s);

	memcpy( (void*)(&sWizeCtx.sProtoCtx.sProtoStats), (void*)(&WizeNet_Uninit), sz);
	eCtl = NETDEV_CTL_GET_STATS;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, args);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	TEST_ASSERT_EQUAL_MEMORY((void*)(&WizeNet_Uninit), aData, sz);

	eCtl = NETDEV_CTL_CLR_STATS;
	memset(aData, 0, sz);
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, 0);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	//TEST_ASSERT_EQUAL_MEMORY(aData, (void*)(&(sWizeCtx.sProtoCtx.sProtoStats)), sz);

	// Check err
	eCtl = NETDEV_CTL_GET_ERR;
	sNetDev.eErrType = NETDEV_ERROR_PROTO;
	sWizeCtx.u8ProtoErr = PROTO_PROTO_UNK_ERR;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, args);
	TEST_ASSERT_EQUAL(PROTO_PROTO_UNK_ERR, i32Ret);

	Wize_Proto_GetStrErr_ExpectAndReturn(PROTO_PROTO_UNK_ERR, proto_err_msg[PROTO_PROTO_UNK_ERR]);
	eCtl = NETDEV_CTL_GET_STR_ERR;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, args);
	TEST_ASSERT_EQUAL(proto_err_msg[PROTO_PROTO_UNK_ERR], ((uint32_t*)aData)[0]);

	eCtl = NETDEV_CTL_CLR_ERR;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, 0);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	TEST_ASSERT_EQUAL(PROTO_SUCCESS, sWizeCtx.u8ProtoErr);

	// Check cfg medium
	eCtl = NETDEV_CTL_CFG_MEDIUM;
	((struct medium_cfg_s*)aData)->eRxChannel = PHY_CH120;
	((struct medium_cfg_s*)aData)->eRxModulation = PHY_WM6400;
	((struct medium_cfg_s*)aData)->eTxChannel = PHY_CH150;
	((struct medium_cfg_s*)aData)->eTxModulation = PHY_WM4800;
	((struct medium_cfg_s*)aData)->eTxPower = PHY_PMAX_minus_6db;
	((struct medium_cfg_s*)aData)->i16TxFreqOffset = -125;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, args);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	TEST_ASSERT_EQUAL_MEMORY(aData, (&sWizeCtx.sMediumCfg), sizeof(struct medium_cfg_s));

	// Check cfg proto
	eCtl = NETDEV_CTL_CFG_PROTO;
	((struct proto_config_s*)aData)->u8NetId = 22;
	((struct proto_config_s*)aData)->u8RecvLenMax = 158;
	((struct proto_config_s*)aData)->u8TransLenMax = 88;
	((struct proto_config_s*)aData)->DwnId[0] = 0;
	((struct proto_config_s*)aData)->DwnId[1] = 10;
	((struct proto_config_s*)aData)->DwnId[2] = 55;
	((struct proto_config_s*)aData)->AppAdm = 58;
	((struct proto_config_s*)aData)->filterDisL2 = 44;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, args);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	TEST_ASSERT_EQUAL_MEMORY(aData, (&sWizeCtx.sProtoCtx), sizeof(struct proto_config_s));

	// Check on phy ok
	eCtl = NETDEV_CTL_PHY_CMD | PHY_CTL_GET_TX_POWER;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, PHY_PMAX_minus_6db);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);

	// Check on phy err
	i32PhyRetCode = PHY_STATUS_ERROR;
	eCtl = NETDEV_CTL_PHY_CMD | PHY_CTL_GET_TX_POWER;
	i32Ret = WizeNet_Ioctl(&sNetDev, eCtl, PHY_PMAX_minus_6db);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	_clean_state_();
}

TEST(WizeCoreNet, test_NetApi_Send)
{
	int32_t i32Ret;
	sNetMsg.u8Type = APP_DATA;
	sNetMsg.u16Id = 1;
	sNetMsg.u8Size = 0;
	sNetMsg.pData = &aData;
	strcpy(aData, "Nothing to say");
	sNetMsg.u8Size = strlen(aData);

	// No error
	i32Ret = WizeNet_Send(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	_clean_state_();

	// Phy error
	i32PhyRetCode = PHY_STATUS_ERROR;
	i32Ret = WizeNet_Send(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_PHY, sNetDev.eErrType);
	_clean_state_();

	// Net error
	sNetDev.eState = NETDEV_STATE_BUSY;
	i32Ret = WizeNet_Send(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_BUSY, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_NONE, sNetDev.eErrType);
	_clean_state_();

	// Proto err
	Wize_ProtoBuild_StopIgnore();
	Wize_ProtoBuild_ExpectAnyArgsAndReturn(PROTO_FAILED);
	//sWizeCtx.sProtoCtx.sProtoConfig.u8TransLenMax = 0;
	i32Ret = WizeNet_Send(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_PROTO, sNetDev.eErrType);
	_clean_state_();
}

TEST(WizeCoreNet, test_NetApi_Recv)
{
	int32_t i32Ret;
	sNetMsg.pData = &aData;

	i32Ret = WizeNet_Recv(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	_clean_state_();

	// Phy error
	i32PhyRetCode = PHY_STATUS_ERROR;
	i32Ret = WizeNet_Recv(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_PHY, sNetDev.eErrType);
	_clean_state_();

	// Net error
	sNetDev.eState = NETDEV_STATE_BUSY;
	i32Ret = WizeNet_Recv(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_BUSY, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_NONE, sNetDev.eErrType);
	_clean_state_();

	// Proto err
	Wize_ProtoExtract_ExpectAnyArgsAndReturn(PROTO_FAILED);
	//sWizeCtx.sProtoCtx.sProtoConfig.u8RecvLenMax = 0;
	i32Ret = WizeNet_Recv(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_PROTO, sNetDev.eErrType);
	_clean_state_();
}

TEST(WizeCoreNet, test_NetApi_Listen)
{
	int32_t i32Ret;
	i32Ret = WizeNet_Listen(&sNetDev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	_clean_state_();

	// Phy error
	i32PhyRetCode = PHY_STATUS_ERROR;
	i32Ret = WizeNet_Listen(&sNetDev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_ERROR, i32Ret);
	TEST_ASSERT_EQUAL(NETDEV_ERROR_PHY, sNetDev.eErrType);
	_clean_state_();

	// Net error
	sNetDev.eState = NETDEV_STATE_BUSY;
	i32Ret = WizeNet_Listen(&sNetDev);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_BUSY, i32Ret);
	_clean_state_();
}

TEST(WizeCoreNet, test_NetApi_CallBack)
{
	int32_t i32Ret;
	sNetMsg.u8Type = APP_DATA;
	sNetMsg.u16Id = 1;
	sNetMsg.u8Size = 0;
	sNetMsg.pData = &aData;
	strcpy(aData, "Nothing to say");
	sNetMsg.u8Size = strlen(aData);

	i32Ret = WizeNet_Send(&sNetDev, &sNetMsg);
	TEST_ASSERT_EQUAL(NETDEV_STATUS_OK, i32Ret);
	_TxDone();
	TEST_ASSERT_EQUAL(NETDEV_STATE_IDLE, sNetDev.eState);
}
