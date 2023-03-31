#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "app_template.h"
#include "wize_app.h"

/******************************************************************************/
/******************************************************************************/

//#define HAS_EXTERNAL_FW_UPDATE 1
#define HAS_LOCAL_FW_UPDATE 1

/******************************************************************************/

admin_ann_fw_info_t sFwAnnInfo;
struct update_ctx_s sUpdateCtx;

static inline update_status_e _dwn_start_(void);
static inline void _dwn_complete_(void);

/******************************************************************************/

void App_Process(uint32_t u32Evt)
{
	uint32_t ret;
	ret = WizeApp_Common(u32Evt);
	// Some "ExtApi_ProcNotify" may have been called into :
	// - "WizeApp_AnnCheckFwInfo" function
	// - "WizeApp_OnDwnBlkRecv" function

	u32Evt &= SES_FLG_SES_MSK;
	if (u32Evt & SES_FLG_ADM_COMPLETE)
	{
		// no error
		if ( !(u32Evt & SES_FLG_ADM_ERROR) )
		{
			/*
			* From here, the ADM_RSP to ADM_CMD (if any) has just been sent.
			*/
			if (ret == ADM_WRITE_PARAM)
			{
				/* Notify "external" that command ADM_WRITE has been completed */
				/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_ADM_WRITE); */
			}
			else if (ret == ADM_ANNDOWNLOAD)
			{
				sUpdateCtx.eUpdateStatus = _dwn_start_();
#ifdef HAS_EXTERNAL_FW_UPDATE
				// If failed notify external
				if (sUpdateCtx.ePendUpdate == UPD_PEND_EXTERNAL)
				{
					if(sUpdateCtx.eUpdateStatus == UPD_STATUS_SES_FAILED)
					{
						/* Notify "external" that failed to start download session */
						/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_DWN_FAILED); */
					}
				}
#endif
			}
		}
		// error
		else
		{
			/* Notify "external" that ADM session failed */
			/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_ADM_FAILED); */
		}
	}
	if (u32Evt & SES_FLG_DWN_COMPLETE)
	{
		if ( sUpdateCtx.ePendUpdate == UPD_PEND_INTERNAL)
		{
			_dwn_complete_();
		}
#ifdef HAS_EXTERNAL_FW_UPDATE
		else if (sUpdateCtx.ePendUpdate == UPD_PEND_EXTERNAL)
		{
			// no error
			if ( !(u32Evt & SES_FLG_DWN_ERROR) )
			{
				/* Notify "external" that download session is completed with success */
				/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_DWN_COMPLETE); */
			}
			// error
			else
			{
				/* Notify "external" that download session is completed with failure */
				/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_DWN_FAILED); */
			}
		}
#endif
		sUpdateCtx.ePendUpdate = UPD_PEND_NONE;
	}
	if (u32Evt & SES_FLG_INST_COMPLETE)
	{
		if (ret != ADM_EXECINSTPING)
		{
			// no error
			if ( !(u32Evt & SES_FLG_INST_ERROR))
			{
				/* Notify "external" that install session is completed with success */
				/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_INST_COMPLETE); */
			}
			// error
			else
			{
				/* Notify "external" that install session is completed with failure */
				/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_INST_FAILED); */
			}
		}
	}
}

void Time_Process(uint32_t u32Evt)
{
	uint32_t ret;
	// Day passed occurs
	if (u32Evt & TIME_FLG_DAY_PASSED)
	{
		ret = WizeApp_Time();

		// Periodic Install
		if (ret & WIZEAPP_INFO_PERIO_INST)
		{
			WizeApp_Install();
		}
		// Back Full Power
		if (ret & WIZEAPP_INFO_FULL_POWER)
		{
			// go back in full power
			uint8_t temp = PHY_PMAX_minus_0db;
			Param_Access(TX_POWER, &temp, 1 );
		}
	}
	// Time correction is requested
	// if(u32Evt & TIME_FLG_TIME_CHANGE) { }

	// Time correction occurs
	// if(u32Evt & TIME_FLG_TIME_ADJ) { }
}


/******************************************************************************/
// Convenient Local functions
/******************************************************************************/

static inline
update_status_e _dwn_start_(void)
{
	/*
	* ADM_RSP was potentially "negative", so check that sUpdateCtx.eErrCode is 0
	*/
	if ( !(sUpdateCtx.eErrCode) )
	{
		// Init dwn storage
		if ( sUpdateCtx.ePendUpdate == UPD_PEND_INTERNAL)
		{
			if ( ImgStore_Init(sFwAnnInfo.u16BlkCnt) )
			{
				// Failed
				return UPD_STATUS_STORE_FAILED;
			}
		}
#ifdef HAS_EXTERNAL_FW_UPDATE
		else if (sUpdateCtx.ePendUpdate == UPD_PEND_EXTERNAL)
		{
			/* Initialize the area to store the external FW block(s) */
			/* YOUR CODE HERE : ExtApi_fw_buffer_init(); */
		}
#endif
		// Start the download session
		if ( WizeApi_Download() != WIZE_API_SUCCESS )
		{
			// failed
			return UPD_STATUS_SES_FAILED;
		}
		else
		{
			return UPD_STATUS_INPROGRESS;
		}
	}
	return UPD_STATUS_UNK;
}

static inline
void _dwn_complete_(void)
{
	if ( ImgStore_IsComplete() )
	{
		if ( ImgStore_Verify(sFwAnnInfo.u32HashSW, 4) )
		{
			// image is corrupted
			sUpdateCtx.eUpdateStatus = UPD_STATUS_CORRUPTED;
		}
		else
		{
			// image is valid
			sUpdateCtx.eUpdateStatus = UPD_STATUS_VALID;
		}
	}
	else
	{
		// image is not complete
		sUpdateCtx.eUpdateStatus = UPD_STATUS_INCOMPLETE;
	}
}

/******************************************************************************/
// WizeCore App hooks functions
/******************************************************************************/

void WizeApp_AnnCheckFwInfo(admin_cmd_anndownload_t *pAnn)
{
	uint32_t u32PrevDwnId = sFwAnnInfo.u32DwnId;
	sFwAnnInfo.u32DwnId = (pAnn->L7DwnId[0] >> 16) | (pAnn->L7DwnId[1]) | (pAnn->L7DwnId[2] << 16);
	sFwAnnInfo.u16SwVerIni = __ntohs( *(uint16_t*)(pAnn->L7SwVersionIni) );
	sFwAnnInfo.u16SwVerTgt = __ntohs( *(uint16_t*)(pAnn->L7SwVersionTarget) );
	sFwAnnInfo.u16DcHwId = __ntohs( *(uint16_t*)(pAnn->L7DcHwId) );
	sFwAnnInfo.u16BlkCnt = __ntohs( *(uint16_t*)(pAnn->L7BlocksCount) );
	sFwAnnInfo.u8DayRepeat = pAnn->L7DayRepeat;
	sFwAnnInfo.u8DeltaSec = pAnn->L7DeltaSec;
	sFwAnnInfo.u32DaysProg = __ntohl( *(uint32_t*)(pAnn->L7DaysProg) );
	sFwAnnInfo.u32HashSW = __ntohl( *(uint32_t*)(pAnn->L7HashSW) );

	/*
	 *  TODO : request to reprogram the same download "pAnn->L7DwnId"
	 *  - Internal FW : don't erase the the image storage area
	 *  - External FW : ??
	 */
	if (sFwAnnInfo.u32DwnId == u32PrevDwnId)
	{
		// request to restart the same download session
		if (sUpdateCtx.eUpdateStatus == UPD_STATUS_INCOMPLETE)
		{
			// image is not complete, so session is permitted
		}
		else if (sUpdateCtx.eUpdateStatus == UPD_STATUS_CORRUPTED)
		{
			// image is corrupted, so erase the image storage area
		}
	}

#ifdef HAS_EXTERNAL_FW_UPDATE
	// Check if it's external or internal FW
	if ( !(sFwAnnInfo.u8DayRepeat & 0x80) )
#endif
	{
		// Internal FW
		sUpdateCtx.ePendUpdate = UPD_PEND_INTERNAL;
		sUpdateCtx.eErrCode = AdmInt_AnnCheckIntFW(&sFwAnnInfo, &sUpdateCtx.eErrParam);
		WizeApp_AnnReady(sUpdateCtx.eErrCode, sUpdateCtx.eErrParam);
	}
#ifdef HAS_EXTERNAL_FW_UPDATE
	else
	{
		// external FW
		sUpdateCtx.ePendUpdate = UPD_PEND_EXTERNAL;
		/* Notify "external" that an ANN_DOWNLOAD is received */
		/* YOUR CODE HERE : ExtApi_ProcNotify(EXT_PEND_ADM_ANN_RECV); */
	}
#endif
}

uint8_t WizeApp_OnDwnBlkRecv(uint16_t u16Id, const uint8_t *pData)
{
	if ( sUpdateCtx.ePendUpdate == UPD_PEND_INTERNAL )
	{
		ImgStore_StoreBlock(u16Id, pData);
	}
#ifdef HAS_EXTERNAL_FW_UPDATE
	else if (sUpdateCtx.ePendUpdate == UPD_PEND_EXTERNAL)
	{
		/* Store the external FW block */
		/* YOUR CODE HERE : ExtApi_fw_buffer_write(u16Id, pData); */
	}
#endif
	return 0;
}

int32_t AdmInt_AnnIsLocalUpdate(void)
{
	if(sUpdateCtx.ePendUpdate == UPD_PEND_NONE)
	{
		return -1;
	}
	else
	{
		// An update already in progress
		return (sFwAnnInfo.u16SwVerTgt & 0x0000FFFF);
	}
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
