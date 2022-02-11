#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(Samples_Parameters);

#include "parameters.h"
#include "parameters_def.h"

#include "parameters_cfg.h"

const uint16_t u16_ParamValueSz = PARAM_DEFAULT_SZ;
const uint8_t u8_ParamAccessCfgSz = PARAM_ACCESS_CFG_SZ;
const uint8_t u8_ParamRestrCfgSz = PARAM_RESTR_CFG_SZ;

uint8_t a_ParamValue[PARAM_DEFAULT_SZ];
//const param_s a_ParamAccess[];
//const restr_s a_ParamRestr[];

TEST_SETUP(Samples_Parameters)
{
	memcpy(a_ParamValue, a_ParamDefault, PARAM_DEFAULT_SZ);
}

TEST_TEAR_DOWN(Samples_Parameters)
{

}

/******************************************************************************/
/* Tests on "Helpers" Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_Param_GetSize_Success)
{
	uint8_t i, u8_v;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_v  = Param_GetSize(i);
		TEST_ASSERT_EQUAL(a_ParamAccess[i].u8_size, u8_v);
	}
}

TEST(Samples_Parameters, test_Param_GetLocAccess_Success)
{
	uint8_t i, u8_v, u8_y;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_v  = Param_GetLocAccess(i);
		u8_y = (a_ParamAccess[i].u8_access >> LOC_ACCESS_POS) & ( LOC_ACCESS_MSK );
		TEST_ASSERT_EQUAL(u8_y, u8_v);
	}
}

TEST(Samples_Parameters, test_Param_GetRemAccess_Success)
{
	uint8_t i, u8_v, u8_y;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_v  = Param_GetRemAccess(i);
		u8_y = (a_ParamAccess[i].u8_access >> REM_ACCESS_POS) & ( REM_ACCESS_MSK );
		TEST_ASSERT_EQUAL(u8_y, u8_v);
	}
}

TEST(Samples_Parameters, test_Param_GetEffect_Success)
{
	uint8_t i, u8_v, u8_y;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_v  = Param_GetEffect(i);
		u8_y = (a_ParamAccess[i].u8_access >> EFF_ACCESS_POS) & ( EFF_ACCESS_MSK );
		TEST_ASSERT_EQUAL(u8_y, u8_v);
	}
}

TEST(Samples_Parameters, test_Param_GetRestrId_Success)
{
	uint8_t i, u8_v;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_v  = Param_GetRestrId(i);
		TEST_ASSERT_EQUAL(a_ParamAccess[i].u8_restId, u8_v);
	}
}

/******************************************************************************/
/* Tests on Local Access Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_LocalReadAccess_Success)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir, u8_y;

	u8_Dir = 0; // Read
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> LOC_ACCESS_POS) & ( LOC_ACCESS_MSK );
		if ( u8_y == RO || u8_y == RW) {
			// Access is granted
			ret = Param_LocalAccess(i, a_Data, u8_Dir);
			TEST_ASSERT_EQUAL(1, ret);
			TEST_ASSERT_EQUAL_MEMORY(Param_GetAddOf(i), a_Data, a_ParamAccess[i].u8_size);
		}
	}
}

TEST(Samples_Parameters, test_LocalWriteAccess_Success)
{
    uint8_t a_Data[128] = {
    		0xDA,
    		0xC0,0xCA,
    		0xDE,0xAD,0xBE,0xEF,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		0xBA,
    		0xDE,0xAD,
    		0xAD,0xAD,0xAB,0xEC,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		0xBA,
    		0xDE,0xAD,
    		0xAD,0xAD,0xAB,0xEC,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		'q', 'w', 'e', 'r', 't', 'y',
    		'r', 'a', 'z', 'o', 'u', 't',
    		'q', 'a', 'l', 'i', 'b', 'a',
    		'a', 'l', 'i', 'b', 'a', 'b',
    		'a', '-', 'e', 't', '-', 'l',
    		'e', 's', '-', '4', '0', '-',
    		'v', 'o', 'e', 'u', 'r', 's',
    		'y', 'o', 'u', 'p', 'i', '*',
    		};
    uint8_t *pS, *pD;
	uint8_t ret, i, u8_Dir, u8_y;
	char str[50] = "";

	u8_Dir = 1; // Write
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> LOC_ACCESS_POS) & ( LOC_ACCESS_MSK );
		if ( u8_y == WO || u8_y == RW) {

			pS = &(a_Data[a_ParamAccess[i].u16_offset]);
			// Write the data
			ret = Param_LocalAccess(i, pS, u8_Dir);
			TEST_ASSERT_EQUAL(1, ret);

			pD = (uint8_t *)Param_GetAddOf(i);
			sprintf(str, "Param Id = %d", i);
			TEST_ASSERT_EQUAL_MEMORY_MESSAGE(pS, pD, a_ParamAccess[i].u8_size, str);

		}
	}
}

TEST(Samples_Parameters, test_LocalReadAccess_Forbidden)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir, u8_y;

	u8_Dir = 0; // Read
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> LOC_ACCESS_POS) & ( LOC_ACCESS_MSK );
		if ( u8_y == NA || u8_y == WO) {
			// Access is forbidden
			ret = Param_LocalAccess(i, a_Data, u8_Dir);
			TEST_ASSERT_EQUAL(0, ret);
		}
	}
}

TEST(Samples_Parameters, test_LocalWriteAccess_Forbidden)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir, u8_y;

	u8_Dir = 1; // Write
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> LOC_ACCESS_POS) & ( LOC_ACCESS_MSK );
		if ( u8_y == NA || u8_y == WO) {
			// Access is forbidden
			ret = Param_LocalAccess(i, a_Data, u8_Dir);
			TEST_ASSERT_EQUAL(0, ret);
		}
	}
	// Check that memory doesn't changed
	TEST_ASSERT_EQUAL_MEMORY(a_ParamDefault, a_ParamValue, PARAM_DEFAULT_SZ);
}

/******************************************************************************/
/* Tests on Remote Access Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_RemoteReadAccess_Success)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir, u8_y;

	u8_Dir = 0; // Read
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> REM_ACCESS_POS) & ( REM_ACCESS_MSK );
		if ( u8_y == RO || u8_y == RW) {
			// Access is granted
			ret = Param_RemoteAccess(i, a_Data, u8_Dir);
			TEST_ASSERT_EQUAL(1, ret);
			TEST_ASSERT_EQUAL_MEMORY(Param_GetAddOf(i), a_Data, a_ParamAccess[i].u8_size);
		}
	}
}

TEST(Samples_Parameters, test_RemoteWriteAccess_Success)
{
    uint8_t a_Data[128] = {
    		0xDA,
    		0xC0,0xCA,
    		0xDE,0xAD,0xBE,0xEF,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		0xBA,
    		0xDE,0xAD,
    		0xAD,0xAD,0xAB,0xEC,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		0xBA,
    		0xDE,0xAD,
    		0xAD,0xAD,0xAB,0xEC,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		'q', 'w', 'e', 'r', 't', 'y',
    		'r', 'a', 'z', 'o', 'u', 't',
    		'q', 'a', 'l', 'i', 'b', 'a',
    		'a', 'l', 'i', 'b', 'a', 'b',
    		'a', '-', 'e', 't', '-', 'l',
    		'e', 's', '-', '4', '0', '-',
    		'v', 'o', 'e', 'u', 'r', 's',
    		'y', 'o', 'u', 'p', 'i', '*',
    		};
    uint8_t *pS, *pD;
	uint8_t ret, i, u8_Dir, u8_y;
	char str[50] = "";

	u8_Dir = 1; // Write
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> REM_ACCESS_POS) & ( REM_ACCESS_MSK );
		if ( u8_y == WO || u8_y == RW) {

			pS = &(a_Data[a_ParamAccess[i].u16_offset]);
			// Write the data
			ret = Param_RemoteAccess(i, pS, u8_Dir);
			TEST_ASSERT_EQUAL(1, ret);

			pD = (uint8_t *)Param_GetAddOf(i);
			sprintf(str, "Param Id = %d", i);
			TEST_ASSERT_EQUAL_MEMORY_MESSAGE(pS, pD, a_ParamAccess[i].u8_size, str);

		}
	}
}

TEST(Samples_Parameters, test_RemoteReadAccess_Forbidden)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir, u8_y;

	u8_Dir = 0; // Read
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> REM_ACCESS_POS) & ( REM_ACCESS_MSK );
		if ( u8_y == NA || u8_y == WO) {
			// Access is forbidden
			ret = Param_LocalAccess(i, a_Data, u8_Dir);
			TEST_ASSERT_EQUAL(0, ret);
		}
	}
}

TEST(Samples_Parameters, test_RemoteWriteAccess_Forbidden)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir, u8_y;

	u8_Dir = 1; // Write
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		u8_y = (a_ParamAccess[i].u8_access >> REM_ACCESS_POS) & ( REM_ACCESS_MSK );
		if ( u8_y == NA || u8_y == WO) {
			// Access is forbidden
			ret = Param_LocalAccess(i, a_Data, u8_Dir);
			TEST_ASSERT_EQUAL(0, ret);
		}
	}
	// Check that memory doesn't changed
	TEST_ASSERT_EQUAL_MEMORY(a_ParamDefault, a_ParamValue, PARAM_DEFAULT_SZ);
}


/******************************************************************************/
/* Tests on Direct Access Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_DirectReadAccess_Success)
{
    uint8_t a_Data[128];
	uint8_t ret, i, u8_Dir;

	u8_Dir = 0; // Read
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
		// Access is granted
		ret = Param_Access(i, a_Data, u8_Dir);
		TEST_ASSERT_EQUAL(1, ret);
		if (a_ParamAccess[i].u8_size > 0) {
			TEST_ASSERT_EQUAL_MEMORY(Param_GetAddOf(i), a_Data, a_ParamAccess[i].u8_size);
		}
	}
}

TEST(Samples_Parameters, test_DirectWriteAccess_Success)
{
    uint8_t a_Data[128] = {
    		0xDA,
    		0xC0,0xCA,
    		0xDE,0xAD,0xBE,0xEF,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		0xBA,
    		0xDE,0xAD,
    		0xAD,0xAD,0xAB,0xEC,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		0xBA,
    		0xDE,0xAD,
    		0xAD,0xAD,0xAB,0xEC,
			0xAD,0xAD,0xA0,0x00,0xDE,0xAD,0xBE,0xEF,
    		'q', 'w', 'e', 'r', 't', 'y',
    		'r', 'a', 'z', 'o', 'u', 't',
    		'q', 'a', 'l', 'i', 'b', 'a',
    		'a', 'l', 'i', 'b', 'a', 'b',
    		'a', '-', 'e', 't', '-', 'l',
    		'e', 's', '-', '4', '0', '-',
    		'v', 'o', 'e', 'u', 'r', 's',
    		'y', 'o', 'u', 'p', 'i', '*',
    		};
    uint8_t *pS, *pD;
	uint8_t ret, i, u8_Dir;
	char str[50] = "";

	u8_Dir = 1; // Write
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++) {
			pS = &(a_Data[a_ParamAccess[i].u16_offset]);
			// Write the data
			ret = Param_Access(i, pS, u8_Dir);
			TEST_ASSERT_EQUAL(1, ret);

			pD = (uint8_t *)Param_GetAddOf(i);
			sprintf(str, "Param Id = %d", i);
			if (a_ParamAccess[i].u8_size > 0) {
				TEST_ASSERT_EQUAL_MEMORY_MESSAGE(pS, pD, a_ParamAccess[i].u8_size, str);
			}
	}
}

/******************************************************************************/
/* Tests on Check Range conformity Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_Param_CheckConformityRange_Success)
{
    uint8_t a_DataMax[10];
    uint8_t a_DataMin[10];
	uint8_t ret, i, u8_restrId, u8_type, u8_sz;
	ret = 0;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++)
	{
		u8_restrId = Param_GetRestrId(i);
		if (u8_restrId != 0)
		{
			u8_type = GET_RESTR_TYPE(a_ParamRestr[u8_restrId].u8_restr);
			u8_sz = GET_RESTR_SZ(a_ParamRestr[u8_restrId].u8_restr);
			if ( u8_type == RESTR_RANGE){
				switch (u8_sz){
					case RESTR_8BITS:
						((uint8_t*)a_DataMax)[0] =  200;
						((uint8_t*)a_DataMin)[0] =  20;
						break;
					case RESTR_16BITS:
						((uint16_t*)a_DataMax)[0] =  2000;
						((uint16_t*)a_DataMin)[0] =  20;
						break;
					case RESTR_32BITS:
						((uint32_t*)a_DataMax)[0] =  200000;
						((uint32_t*)a_DataMin)[0] =  20;
						break;
					case RESTR_64BITS:
						((uint64_t*)a_DataMax)[0] =  200000000000;
						((uint64_t*)a_DataMin)[0] =  20;
						break;
					default:
						TEST_ASSERT_EQUAL(1, ret);
						break;
					}
				// Check the Max
				ret = Param_CheckConformity(i, a_DataMax);
				TEST_ASSERT_EQUAL(1, ret);
				// Check the min
				ret = Param_CheckConformity(i, a_DataMin);
				TEST_ASSERT_EQUAL(1, ret);
			}
		}
	}
}

TEST(Samples_Parameters, test_Param_CheckConformityRange_Forbidden)
{
    uint8_t a_DataMax[10];
    uint8_t a_DataMin[10];
	uint8_t ret, i, u8_restrId, u8_type, u8_sz;
	ret = 0;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++)
	{
		u8_restrId = Param_GetRestrId(i);
		if (u8_restrId != 0)
		{
			u8_type = GET_RESTR_TYPE(a_ParamRestr[u8_restrId].u8_restr);
			u8_sz = GET_RESTR_SZ(a_ParamRestr[u8_restrId].u8_restr);
			if ( u8_type == RESTR_RANGE){
				switch (u8_sz){
					case RESTR_8BITS:
						((uint8_t*)a_DataMax)[0] =  255;
						((uint8_t*)a_DataMin)[0] =  2;
						break;
					case RESTR_16BITS:
						((uint16_t*)a_DataMax)[0] =  3000;
						((uint16_t*)a_DataMin)[0] =  2;
						break;
					case RESTR_32BITS:
						((uint32_t*)a_DataMax)[0] =  300000;
						((uint32_t*)a_DataMin)[0] =  2;
						break;
					case RESTR_64BITS:
						((uint64_t*)a_DataMax)[0] =  300000000000;
						((uint64_t*)a_DataMin)[0] =  2;
						break;
					default:
						TEST_ASSERT_EQUAL(1, ret);
						break;
					}
				// Check the Max
				ret = Param_CheckConformity(i, a_DataMax);
				TEST_ASSERT_EQUAL(0, ret);
				// Check the min
				ret = Param_CheckConformity(i, a_DataMin);
				TEST_ASSERT_EQUAL(0, ret);
			}
		}
	}
}

/******************************************************************************/
/* Tests on Check Enum conformity Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_Param_CheckConformityEnum_Success)
{
    uint8_t a_DataMax[10];
    uint8_t a_DataMin[10];
	uint8_t ret, i, u8_restrId, u8_type, u8_sz;
	ret = 0;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++)
	{
		u8_restrId = Param_GetRestrId(i);
		if (u8_restrId != 0)
		{
			u8_type = GET_RESTR_TYPE(a_ParamRestr[u8_restrId].u8_restr);
			u8_sz = GET_RESTR_SZ(a_ParamRestr[u8_restrId].u8_restr);
			if ( u8_type == RESTR_ENUM){
				switch (u8_sz){
					case RESTR_8BITS:
						((uint8_t*)a_DataMax)[0] =  144;
						((uint8_t*)a_DataMin)[0] =  2;
						break;
					case RESTR_16BITS:
						((uint16_t*)a_DataMax)[0] =  2000;
						((uint16_t*)a_DataMin)[0] =  2;
						break;
					case RESTR_32BITS:
						((uint32_t*)a_DataMax)[0] =  200000;
						((uint32_t*)a_DataMin)[0] =  2;
						break;
					case RESTR_64BITS:
						((uint64_t*)a_DataMax)[0] =  200000000000;
						((uint64_t*)a_DataMin)[0] =  2;
						break;
					default:
						TEST_ASSERT_EQUAL(1, ret);
						break;
					}
				// Check the Max
				ret = Param_CheckConformity(i, a_DataMax);
				TEST_ASSERT_EQUAL(1, ret);
				// Check the min
				ret = Param_CheckConformity(i, a_DataMin);
				TEST_ASSERT_EQUAL(1, ret);
			}
		}
	}
}

TEST(Samples_Parameters, test_Param_CheckConformityEnum_Forbidden)
{
    uint8_t a_DataMax[10];
    uint8_t a_DataMin[10];
	uint8_t ret, i, u8_restrId, u8_type, u8_sz;
	ret = 0;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++)
	{
		u8_restrId = Param_GetRestrId(i);
		if (u8_restrId != 0)
		{
			u8_type = GET_RESTR_TYPE(a_ParamRestr[u8_restrId].u8_restr);
			u8_sz = GET_RESTR_SZ(a_ParamRestr[u8_restrId].u8_restr);
			if ( u8_type == RESTR_ENUM){
				switch (u8_sz){
					case RESTR_8BITS:
						((uint8_t*)a_DataMax)[0] =  244;
						((uint8_t*)a_DataMin)[0] =  0;
						break;
					case RESTR_16BITS:
						((uint16_t*)a_DataMax)[0] =  200;
						((uint16_t*)a_DataMin)[0] =  1;
						break;
					case RESTR_32BITS:
						((uint32_t*)a_DataMax)[0] =  100000;
						((uint32_t*)a_DataMin)[0] =  6;
						break;
					case RESTR_64BITS:
						((uint64_t*)a_DataMax)[0] =  5000000000;
						((uint64_t*)a_DataMin)[0] =  5;
						break;
					default:
						TEST_ASSERT_EQUAL(1, ret);
						break;
					}
				// Check the Max
				ret = Param_CheckConformity(i, a_DataMax);
				TEST_ASSERT_EQUAL(0, ret);
				// Check the min
				ret = Param_CheckConformity(i, a_DataMin);
				TEST_ASSERT_EQUAL(0, ret);
			}
		}
	}
}
/******************************************************************************/
/* Tests on Check Modulo conformity Functions */
/******************************************************************************/
TEST(Samples_Parameters, test_Param_CheckConformityModulo_Success)
{
    uint8_t a_Data[20];
	uint8_t ret, i, u8_restrId, u8_type, u8_sz;
	ret = 0;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++)
	{
		u8_restrId = Param_GetRestrId(i);
		if (u8_restrId != 0)
		{
			u8_type = GET_RESTR_TYPE(a_ParamRestr[u8_restrId].u8_restr);
			u8_sz = GET_RESTR_SZ(a_ParamRestr[u8_restrId].u8_restr);
			if ( u8_type == RESTR_MODULO){
				switch (u8_sz){
					case RESTR_8BITS:
						((uint8_t*)a_Data)[0] =  25;
						break;
					case RESTR_16BITS:
						((uint16_t*)a_Data)[0] =  2500;
						break;
					case RESTR_32BITS:
						((uint32_t*)a_Data)[0] =  25000000;
						break;
					case RESTR_64BITS:
						((uint64_t*)a_Data)[0] =  2500000000;
						break;
					default:
						TEST_ASSERT_EQUAL(1, ret);
						break;
					}

				ret = Param_CheckConformity(i, a_Data);
				TEST_ASSERT_EQUAL(1, ret);
			}
		}
	}
}

TEST(Samples_Parameters, test_Param_CheckConformityModulo_Forbidden)
{
    uint8_t a_Data[20];
	uint8_t ret, i, u8_restrId, u8_type, u8_sz;
	ret = 0;
	for (i=0; i < PARAM_ACCESS_CFG_SZ; i++)
	{
		u8_restrId = Param_GetRestrId(i);
		if (u8_restrId != 0)
		{
			u8_type = GET_RESTR_TYPE(a_ParamRestr[u8_restrId].u8_restr);
			u8_sz = GET_RESTR_SZ(a_ParamRestr[u8_restrId].u8_restr);
			if ( u8_type == RESTR_MODULO){
				switch (u8_sz){
					case RESTR_8BITS:
						((uint8_t*)a_Data)[0] =  26;
						break;
					case RESTR_16BITS:
						((uint16_t*)a_Data)[0] =  2600;
						break;
					case RESTR_32BITS:
						((uint32_t*)a_Data)[0] =  26000000;
						break;
					case RESTR_64BITS:
						((uint64_t*)a_Data)[0] =  2600000000;
						break;
					default:
						TEST_ASSERT_EQUAL(1, ret);
						break;
					}
				ret = Param_CheckConformity(i, a_Data);
				TEST_ASSERT_EQUAL(0, ret);
			}
		}
	}
}
