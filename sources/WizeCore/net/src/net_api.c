/**
  * @file: net_api.c
  * @brief: This file implement all required function to access network device (aka. netdev_t).
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2020/06/05[GBI]
  * Initial version
  * 2.0.0 : 2021/10/24[GBI]
  * Replace Setter/Getter function by one Ioctl function
  *
  */
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "net_api_private.h"

// Internal
static int32_t _check_idle_state(netdev_t* pNetdev);

// event callback from phy
static void _evt_cb(void *p_CbParam, uint32_t evt);

/*!
 * @brief  This function setup the netdev_t device
 *
 * @param [in] pNetdev  Pointer on netdev_t device
 * @param [in] pWizeCtx Pointer on network context
 * @param [in] pPhydev  Pointer on the PHY device context
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
int32_t WizeNet_Setup(netdev_t* pNetdev, wize_net_t* pWizeCtx, phydev_t *pPhydev)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
    if (pNetdev && pWizeCtx && pPhydev)
    {
    	pNetdev->pCtx = pWizeCtx;
    	pNetdev->pPhydev = pPhydev;
    	pNetdev->eState = NETDEV_STATE_UNKWON;
    	i32Ret = 0;
    }
	return i32Ret;
}

/*!
 * @brief  This function initialize the netdev_t and phy devices
 *
 * @param [in] pNetdev   Pointer on netdev_t device
 * @param [in] pfcbEvent Event call-back to upper layer (still in interrupt)
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
int32_t WizeNet_Init(netdev_t* pNetdev, netdev_evt_cb_t pfcbEvent)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
    if (pNetdev && pfcbEvent && pNetdev->pPhydev)
    {
        const phy_if_t *pIf = pNetdev->pPhydev->pIf;
        if ( pIf->pfInit(pNetdev->pPhydev) != PHY_STATUS_OK)
        {
            pNetdev->eState = NETDEV_STATE_UNKWON;
            pNetdev->eErrType = NETDEV_ERROR_PHY;
        }
        else {
			pNetdev->eState = NETDEV_STATE_IDLE;
			pNetdev->eErrType = NETDEV_ERROR_NONE;
			pNetdev->cbEvent = pfcbEvent;
			pNetdev->pPhydev->bPreSyncOn = 1;
			pNetdev->pPhydev->pfEvtCb = &_evt_cb;
			pNetdev->pPhydev->pCbParam = (void*)pNetdev;
			i32Ret = NETDEV_STATUS_OK;
        }
    }
	return i32Ret;
}

/*!
 * @brief  This function de-initialize the netdev_t and phy devices.
 *
 * @param [in] pNetdev Pointer on netdev_t device
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
int32_t WizeNet_Uninit(netdev_t* pNetdev)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
    if (pNetdev && pNetdev->pPhydev)
    {
        const phy_if_t *pIf = pNetdev->pPhydev->pIf;
        if ( pIf->pfUnInit(pNetdev->pPhydev) == PHY_STATUS_OK )
        {
        	i32Ret = NETDEV_STATUS_OK;
        }
        pNetdev->eState = NETDEV_STATE_UNKWON;
    }
	return i32Ret;
}

/*!
 * @brief  This function send the given message.
 *
 * @param [in] pNetdev Pointer on netdev_t device
 * @param [in] pNetMsg Pointer on structure that hold the message
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
int32_t WizeNet_Send(netdev_t* pNetdev, net_msg_t *pNetMsg)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
	wize_net_t* pCtx;
	struct medium_cfg_s* pConfig;
	const phy_if_t* pIf;
	if (pNetMsg && pNetMsg->pData)
	{
		i32Ret = _check_idle_state(pNetdev);
		if ( i32Ret == NETDEV_STATUS_OK )
		{
			i32Ret = NETDEV_STATUS_ERROR;
			pIf = pNetdev->pPhydev->pIf;
			pCtx = (wize_net_t*)pNetdev->pCtx;
			pConfig = &(pCtx->sMediumCfg);

			pNetdev->eState = NETDEV_STATE_BUSY;

			pCtx->sProtoCtx.pBuffer = pCtx->aSendBuff;
			pCtx->u8ProtoErr = Wize_ProtoBuild(&(pCtx->sProtoCtx), pNetMsg);
			if ( !(pCtx->u8ProtoErr) )
			{
				uint8_t u8FrmSize = pCtx->sProtoCtx.u8Size;
				if(pNetdev->pPhydev->bCrcOn == 1) {
					u8FrmSize -= 2; // remove CRC
				}
				
				pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_SET_TX_POWER, (uint32_t)pConfig->eTxPower);
				pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_SET_TX_FREQ_OFF, (uint32_t)pConfig->i16TxFreqOffset);

				if (pIf->pfSetSend(
						pNetdev->pPhydev,
						&(pCtx->aSendBuff[1]), // to remove LEN field
						(u8FrmSize)
						))
				{
					pNetdev->eErrType = NETDEV_ERROR_PHY;
					pNetdev->eState = NETDEV_STATE_ERROR;
					return i32Ret;
				}
				// FIXME : Noise measurement rise an interrupt
				uint8_t u8Noise = 0; //pIf->pfGetNoise(pNetdev->pPhydev);
				if ( pIf->pfTx(pNetdev->pPhydev, pConfig->eTxChannel, pConfig->eTxModulation) )
				{
					pNetdev->eErrType = NETDEV_ERROR_PHY;
					pNetdev->eState = NETDEV_STATE_ERROR;
					return i32Ret;
				}
				Wize_ProtoStats_TxUpdate(&(pCtx->sProtoCtx), pCtx->u8ProtoErr, u8Noise);
				i32Ret = NETDEV_STATUS_OK;
			}
			else {
				pNetdev->eState = NETDEV_STATE_ERROR;
				pNetdev->eErrType = NETDEV_ERROR_PROTO;
			}
		}
	}
	// else { } // NULL pointer
	return i32Ret;
}

/*!
 * @brief  This function get the received message
 *
 * @param [in] pNetdev Pointer on netdev_t device
 * @param [in] pNetMsg Pointer on structure that will hold the message
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
int32_t WizeNet_Recv(netdev_t* pNetdev, net_msg_t *pNetMsg)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
	wize_net_t* pCtx;
	const phy_if_t* pIf;

	if (pNetMsg && pNetMsg->pData)
	{
		i32Ret = _check_idle_state(pNetdev);
		if ( i32Ret == NETDEV_STATUS_OK )
		{
			pIf = pNetdev->pPhydev->pIf;
			pCtx = (wize_net_t*)pNetdev->pCtx;

			// _aRecvBuff must be protected
			if ( pIf->pfGetRecv(pNetdev->pPhydev, &(pCtx->aRecvBuff[1]), &(pCtx->sProtoCtx.u8Size) ) )
			{
				pNetdev->eErrType = NETDEV_ERROR_PHY;
				pNetdev->eState = NETDEV_STATE_ERROR;
				i32Ret = NETDEV_STATUS_ERROR;
				return i32Ret;
			}
			pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_RSSI, (uint32_t)(&pNetMsg->u8Rssi));

			// now the PHY can be IDLE or READY
			pCtx->sProtoCtx.pBuffer = pCtx->aRecvBuff;
			pCtx->u8ProtoErr = Wize_ProtoExtract(&(pCtx->sProtoCtx), pNetMsg);
			if ( !(pCtx->u8ProtoErr) )
			{
				i32Ret = NETDEV_STATUS_OK;
			}
			else {
				// Wize stack error
				pNetdev->eState = NETDEV_STATE_ERROR;
				pNetdev->eErrType = NETDEV_ERROR_PROTO;
				i32Ret = NETDEV_STATUS_ERROR;
			}
			Wize_ProtoStats_RxUpdate(&(pCtx->sProtoCtx), pCtx->u8ProtoErr, pNetMsg->u8Rssi);
		}
	}
	// else { } // NULL pointer
	return i32Ret;
}

/*!
 * @brief  This function open a listen window
 *
 * @param [in] pNetdev Pointer on netdev_t device
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
int32_t WizeNet_Listen(netdev_t* pNetdev)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
	wize_net_t* pCtx;
	const phy_if_t* pIf;
	struct medium_cfg_s* pConfig;

	i32Ret = _check_idle_state(pNetdev);
	if ( i32Ret == NETDEV_STATUS_OK )
	{
		pIf = pNetdev->pPhydev->pIf;
		pCtx = (wize_net_t*)pNetdev->pCtx;
		pConfig = &(pCtx->sMediumCfg);
		if ( pIf->pfRx(pNetdev->pPhydev, pConfig->eRxChannel, pConfig->eRxModulation) )
		{
			pNetdev->eErrType = NETDEV_ERROR_PHY;
			pNetdev->eState = NETDEV_STATE_ERROR;
			i32Ret = NETDEV_STATUS_ERROR;
		}
		else {
			pNetdev->eState = NETDEV_STATE_BUSY;
			pNetdev->pPhydev->bPreSyncOn = 1;
		    i32Ret = NETDEV_STATUS_OK;
		}
	}
	// else { } // NULL pointer
	return i32Ret;
}

/*!
 * @brief  This function Get/Set internal configuration variable
 *
 * @param [in]     pNetdev Pointer on netdev_t device
 * @param [in]     eCtl    Id of configuration variable to get/set (see netdev_ctl_e)
 * @param [in,out] args    scalar or pointer that hold the value to set/get
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 */
int32_t WizeNet_Ioctl(netdev_t* pNetdev, uint32_t eCtl, uint32_t args)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
	const phy_if_t* pIf;
	wize_net_t *pCtx;
	int32_t ret;
	if (pNetdev )
	{
		pIf = pNetdev->pPhydev->pIf;
		pCtx = (wize_net_t*)pNetdev->pCtx;
		ret = 0;

		if (eCtl & NETDEV_CTL_PHY_CMD)
		{
			// FIXME : how to pass argument

			uint32_t phy_ctl = eCtl & ~(NETDEV_CTL_PHY_CMD);
			if(eCtl > PHY_CTL_CMD)
			{
				phy_ctl = (phy_ctl_e)args;
			}

			ret = pIf->pfIoctl(pNetdev->pPhydev, (phy_ctl_e)(phy_ctl), args);
			if ( ret )
			{
				pNetdev->eErrType = NETDEV_ERROR_PHY;
				pNetdev->eState = NETDEV_STATE_ERROR;
			}
			else {
				pNetdev->eErrType = NETDEV_ERROR_NONE;
				pNetdev->eState = NETDEV_STATE_IDLE;
				i32Ret = NETDEV_STATUS_OK;
			}
		}
		else if (eCtl > _NETDEV_CTL_ERR_)
		{
			i32Ret = NETDEV_STATUS_OK;

			switch (eCtl)
			{
				case NETDEV_CTL_GET_STATS:
					if( (void*)args == NULL)
					{
						i32Ret = NETDEV_STATUS_ERROR;
						break;
					}
					memcpy((void*)args, &(pCtx->sProtoCtx.sProtoStats), sizeof(net_stats_t));
					break;
				case NETDEV_CTL_CLR_STATS:
					Wize_ProtoStats_RxClear(&(pCtx->sProtoCtx));
					Wize_ProtoStats_TxClear(&(pCtx->sProtoCtx));
					break;
				case NETDEV_CTL_CLR_ERR:
					switch (pNetdev->eErrType)
					{
						case NETDEV_ERROR_PHY:
							if( (void*)args == NULL)
							{
								i32Ret = NETDEV_STATUS_ERROR;
								break;
							}
							//i32Ret = pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_STR_ERR, (uint32_t)(&args));
							i32Ret = pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_STR_ERR, (args));
							break;
						case NETDEV_ERROR_PROTO:
							pCtx->u8ProtoErr = 0;
							break;
						case NETDEV_ERROR_NET:
						default:
							break;
					}
					pNetdev->eErrType = NETDEV_ERROR_NONE;
					pNetdev->eState = NETDEV_STATE_IDLE;
					break;
				case NETDEV_CTL_GET_ERR:
					switch (pNetdev->eErrType)
					{
						case NETDEV_ERROR_PHY:
							if( (void*)args == NULL)
							{
								i32Ret = NETDEV_STATUS_ERROR;
								break;
							}
							//i32Ret = pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_ERR, (uint32_t)(&args));
							i32Ret = pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_STR_ERR, (args));
							break;
						case NETDEV_ERROR_PROTO:
							i32Ret = pCtx->u8ProtoErr;
							break;
						case NETDEV_ERROR_NET:
						default:
							i32Ret = PROTO_FAILED;
							break;
					}
					break;
				case NETDEV_CTL_GET_STR_ERR:
					if( (void*)args == NULL)
					{
						i32Ret = NETDEV_STATUS_ERROR;
						break;
					}
					switch (pNetdev->eErrType)
					{
						case NETDEV_ERROR_PHY:
							//i32Ret = pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_STR_ERR, (uint32_t)(&args));
							i32Ret = pIf->pfIoctl(pNetdev->pPhydev, PHY_CTL_GET_STR_ERR, (args));
							break;
						case NETDEV_ERROR_PROTO:
							*((uint32_t*)args) = (uint32_t)Wize_Proto_GetStrErr(pCtx->u8ProtoErr);
									//(uint32_t)wize_err_msg[pCtx->u8ProtoErr];
							break;
						case NETDEV_ERROR_NET:
						default:
							*((uint32_t*)args) = (uint32_t)Wize_Proto_GetStrErr(PROTO_FAILED);
									//(uint32_t)wize_err_msg[PROTO_FAILED];
							break;
					}
					break;
				default:
					break;
			}
		}
		else
		{
			i32Ret = _check_idle_state(pNetdev);
			if ( i32Ret == NETDEV_STATUS_OK)
			{
				switch (eCtl)
				{
					/***********************************************************/
					case NETDEV_CTL_CFG_MEDIUM:
						if( (void*)args == NULL)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						memcpy(&(pCtx->sMediumCfg), (struct medium_cfg_s*)args, sizeof(struct medium_cfg_s));
						break;

					case NETDEV_CTL_SET_UPLINK_CH:
						if(args >= PHY_NB_CH)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						pCtx->sMediumCfg.eTxChannel = (phy_chan_e)args;
						break;
					case NETDEV_CTL_SET_UPLINK_MOD:
						if(args >= PHY_NB_MOD)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						pCtx->sMediumCfg.eTxModulation = (phy_mod_e)args;
						break;
					case NETDEV_CTL_SET_DWLINK_CH:
						if(args >= PHY_NB_CH)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						pCtx->sMediumCfg.eRxChannel = (phy_chan_e)args;
						break;
					case NETDEV_CTL_SET_DWLINK_MOD:
						if(args >= PHY_NB_MOD)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						pCtx->sMediumCfg.eRxModulation = (phy_mod_e)args;
						break;
					case NETDEV_CTL_SET_FOFFSET:
						pCtx->sMediumCfg.i16TxFreqOffset = (int16_t)args;
						break;
					case NETDEV_CTL_SET_PWR:
						if(args >= PHY_NB_PWR)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						pCtx->sMediumCfg.eTxPower = (phy_power_e)args;
						break;

					/***********************************************************/
					case NETDEV_CTL_CFG_PROTO:
						if( (void*)args == NULL)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						memcpy(&(pCtx->sProtoCtx.sProtoConfig), (struct proto_config_s*)args, sizeof(struct proto_config_s));
						break;

					case NETDEV_CTL_SET_TRANSLEN:
						pCtx->sProtoCtx.sProtoConfig.u8TransLenMax = (uint8_t)args;
						break;
					case NETDEV_CTL_SET_RECVLEN:
						pCtx->sProtoCtx.sProtoConfig.u8RecvLenMax = (uint8_t)args;
						break;
					case NETDEV_CTL_SET_NETWID:
						pCtx->sProtoCtx.sProtoConfig.u8NetId = (uint8_t)args;
						break;
					case NETDEV_CTL_SET_DWNID:
						pCtx->sProtoCtx.sProtoConfig.DwnId[0] = (uint8_t)(args >> 16);
						pCtx->sProtoCtx.sProtoConfig.DwnId[1] = (uint8_t)(args >> 8);
						pCtx->sProtoCtx.sProtoConfig.DwnId[2] = (uint8_t)(args );
						break;

					/***********************************************************/
					case NETDEV_CTL_SET_DEVID:
						if( (void*)args == NULL)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						memcpy(pCtx->sProtoCtx.aDeviceManufID, (uint8_t*)args, MFIELD_SZ);
						memcpy(pCtx->sProtoCtx.aDeviceAddr, &(((uint8_t*)args)[MFIELD_SZ]), AFIELD_SZ);
						break;
					case NETDEV_CTL_GET_DEVID:
						if( (void*)args == NULL)
						{
							i32Ret = NETDEV_STATUS_ERROR;
							break;
						}
						memcpy((uint8_t*)args, pCtx->sProtoCtx.aDeviceManufID, MFIELD_SZ);
						memcpy(&(((uint8_t*)args)[MFIELD_SZ]), pCtx->sProtoCtx.aDeviceAddr, AFIELD_SZ);
						break;
					/***********************************************************/
					default:
						break;
				}
			}
		}
	}
	return i32Ret;
}

/******************************************************************************/
/******************************************************************************/
// Private internal function

/*!
 * @static
 * @brief  This function check if the device is in IDLE state
 *
 * @param [in]  pNetdev Pointer on netdev_t device
 *
 * @retval
 * @li @link netdev_status_e::NETDEV_STATUS_OK @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_ERROR @endlink
 * @li @link netdev_status_e::NETDEV_STATUS_BUSY @endlink
 *
 */
static int32_t _check_idle_state(netdev_t* pNetdev)
{
	if (pNetdev == NULL)
	{
		return NETDEV_STATUS_ERROR;
	}
	// Idle
	if ( pNetdev->eState & NETDEV_STATE_IDLE )
	{
		return NETDEV_STATUS_OK;
	}
	// Busy
	else if (pNetdev->eState & NETDEV_STATE_BUSY)
	{
		return NETDEV_STATUS_BUSY;
	}
	// Error or Unknown
	else
	{
		return NETDEV_STATUS_ERROR;
	}
}

/*!
 * @static
 * @brief  Callback function, from Phy to Higher level (still in interrupt handler)
 *
 * @param [in] p_CbParam Pointer on netdev_t device structure
 * @param [in] evt       Event from lower PHY layer
 *
 * @return      Status
 *
 */
static void _evt_cb(void *p_CbParam, uint32_t evt)
{
	netdev_t* pNetdev = (netdev_t*)p_CbParam;
	netdev_evt_e eNetEvt;
	if (pNetdev)
	{
		switch(evt) {
			case PHYDEV_EVT_RX_COMPLETE:
				eNetEvt = NETDEV_EVT_RX_COMPLETE;
				pNetdev->eState = NETDEV_STATE_IDLE;
				break;
			case PHYDEV_EVT_TX_COMPLETE:
				eNetEvt = NETDEV_EVT_TX_COMPLETE;
				pNetdev->eState = NETDEV_STATE_IDLE;
				break;
			case PHYDEV_EVT_RX_STARTED:
				eNetEvt = NETDEV_EVT_RX_STARTED;
				break;
			case PHYDEV_EVT_ERROR:
				eNetEvt = NETDEV_EVT_ERROR;
				pNetdev->eState = NETDEV_STATE_ERROR;
				break;
			default:
				eNetEvt = NETDEV_EVT_NONE;
				break;
		}
		if(pNetdev->cbEvent){
			pNetdev->cbEvent(eNetEvt);
		}
	}
}

#ifdef __cplusplus
}
#endif
