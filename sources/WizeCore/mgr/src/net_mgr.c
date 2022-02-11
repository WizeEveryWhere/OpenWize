/**
  * @file: net_mgr.c
  * @brief: This file implement functions to manage network device in a multi-thread environment..
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
  * 1.0.0 : 2020/09/28[GBI]
  * Initial version
  * 2.0.0 : 2021/10/19[GBI]
  * Integrate a more complete management for sending and listening.
  *
  */
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "net_mgr.h"

#include "rtos_macro.h"
#include "logger.h"
/******************************************************************************/

#define NET_DEV_ACQUIRE_TIMEOUT() 5
#define NET_MGR_ACQUIRE_TIMEOUT() 10
#define NET_MGR_EVT_TIMEOUT() 0xFFFFFFFF

static void _net_mgr_main_(void const * argument);
static void _net_mgr_evtCb_(uint32_t evt);
static inline void _net_mgr_notify_caller_(uint32_t evt);
static uint32_t _net_mgr_fsm_(netdev_t *pNetDev, uint32_t u32Evt);
static int32_t _net_mgr_send_with_retry_(netdev_t *pNetDev, net_msg_t *pxNetMsg, uint8_t u8Retry);
static int32_t _net_mgr_listen_with_retry_(netdev_t *pNetDev, uint8_t u8Retry);
static int32_t _net_mgr_error_(netdev_t *pNetDev);
static int32_t _net_mgr_try_abort_(netdev_t *pNetDev);

static struct wize_ctx_s sWizeCtx;
static netdev_t sNetDev;

// Wize Net Task, Mutex,
#define NET_MGR_TASK_STACK_SIZE 300
#define NET_MGR_TASK_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+2)
SYS_TASK_CREATE_DEF(netmgr, NET_MGR_TASK_STACK_SIZE, NET_MGR_TASK_PRIORITY);
SYS_MUTEX_CREATE_DEF(netmgr);
SYS_EVENT_CREATE_DEF(netmgr);

//SYS_MUTEX_CREATE_DEF(netdev);
SYS_BINSEM_CREATE_DEF(netdev);

#define _NET_EXPAND_TMO_MSK_ 0x10
#define _NET_REARM_LISTEN_ 0x20 /**< Ready for listen again */

/******************************************************************************/

/*!
 * @ingroup Sources
 * @{
 * @ingroup Wize
 * @{
 * @ingroup NetMgr
 * @{
 *
 */


/*!
 * @brief This function setup the NetMgr module
 *
 * @return None
 */
void NetMgr_Setup(phydev_t *pPhyDev, wize_net_t *pWizeNet)
{
	int32_t i32Ret = -1;

	//sNetDev.hLock = SYS_MUTEX_CREATE_CALL(netdev);
	sNetDev.hLock = SYS_BINSEM_CREATE_CALL(netdev);
	assert(sNetDev.hLock);

	sWizeCtx.hMutex = SYS_MUTEX_CREATE_CALL(netmgr);
	assert(sWizeCtx.hMutex);


	sWizeCtx.hEvents = SYS_EVENT_CREATE_CALL(netmgr);
	assert(sWizeCtx.hEvents);
	sWizeCtx.hTask = SYS_TASK_CREATE_CALL(netmgr, _net_mgr_main_, NULL);
	assert(sWizeCtx.hTask);

	assert(pPhyDev);
	assert(pWizeNet);

	i32Ret = WizeNet_Setup(&sNetDev, pWizeNet, pPhyDev);
	assert(!i32Ret);
	sWizeCtx.hCaller = NULL;

	sWizeCtx.u8RecvRetries = 3;
	sWizeCtx.u8TransRetries = 3;
}

/*!
 * @brief This function initialize the NetMgr module
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Init(void)
{
	int32_t eStatus = NET_STATUS_ERROR;
	if (sWizeCtx.hTask && sNetDev.pCtx && sNetDev.pPhydev)
	{
		if ( TimeEvt_TimerInit(&(sWizeCtx.sTimeOut), sWizeCtx.hTask, TIMEEVT_CFG_ONESHOT) == 0)
		{
			eStatus = WizeNet_Init(&sNetDev, &_net_mgr_evtCb_);
		}
		sWizeCtx.i16ExpandTmo = 900;
	}
	return eStatus;
}

/*!
 * @brief This function de-initialize the NetMgr module
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Uninit(void)
{
	int32_t eStatus = NET_STATUS_ERROR;
	if (sWizeCtx.hTask && sNetDev.pCtx && sNetDev.pPhydev)
	{
		TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
		eStatus = WizeNet_Uninit(&sNetDev);
	}
	return eStatus;
}
/******************************************************************************/

/*!
 * @brief This function open (acquire) the device
 *
 * @param [in] hTaskToNotify Task handle which will get events back from NetMgr.
 *                           If NULL, then the caller task will be notified.
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Open(void *hTaskToNotify)
{
	// try to acquire the Net Mgr
	if ( xSemaphoreTake( sWizeCtx.hMutex, NET_MGR_ACQUIRE_TIMEOUT()) == 0 )
	{
		return NET_STATUS_BUSY;
	}
	// Try to Init device
	if (NetMgr_Init() != NET_STATUS_OK )
	{
		xSemaphoreGive(sWizeCtx.hMutex);
		return NET_STATUS_ERROR;
	}
	if (hTaskToNotify)
	{
		sWizeCtx.hCaller = hTaskToNotify;
	}
	else
	{
		sWizeCtx.hCaller = xTaskGetCurrentTaskHandle( );
	}
	xSemaphoreGive(sNetDev.hLock);
	return NET_STATUS_OK;
}

/*!
 * @brief This function release the device
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Close(void)
{
	// check if caller own the NetMgr mutex
	if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
	{
		NetMgr_Uninit();
		sWizeCtx.hCaller = NULL;
		xSemaphoreGive(sWizeCtx.hMutex);
		return NET_STATUS_OK;
	}
	return NET_STATUS_BUSY;
}

/*!
 * @brief This function set the uplink channel and modulation
 *
 * @param [in] eChannel Channel to set
 * @param [in] eMod     Modulation to set
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_SetUplink(phy_chan_e eChannel, phy_mod_e eMod)
{
	int32_t eStatus = NET_STATUS_BUSY;
	// check if caller own the NetMgr mutex
	if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
	{
		// try to acquire the Net Dev
		if ( xSemaphoreTake( sNetDev.hLock, NET_DEV_ACQUIRE_TIMEOUT()) )
		{
			eStatus = WizeNet_Ioctl(&sNetDev, NETDEV_CTL_SET_UPLINK_CH, (uint32_t)eChannel);
			if ( eStatus == NETDEV_STATUS_OK)
			{
				eStatus = WizeNet_Ioctl(&sNetDev, NETDEV_CTL_SET_UPLINK_MOD, (uint32_t)eMod);
			}
			xSemaphoreGive(sNetDev.hLock);
		}
	}
	return eStatus;
}

/*!
 * @brief This function set the downlink channel and modulation
 *
 * @param [in] eChannel Channel to set
 * @param [in] eMod     Modulation to set
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_SetDwlink(phy_chan_e eChannel, phy_mod_e eMod)
{
	int32_t eStatus = NET_STATUS_BUSY;
	// check if caller own the NetMgr mutex
	if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
	{
		// try to acquire the Net Dev
		if ( xSemaphoreTake( sNetDev.hLock, NET_DEV_ACQUIRE_TIMEOUT()) )
		{
			eStatus = WizeNet_Ioctl(&sNetDev, NETDEV_CTL_SET_DWLINK_CH, (uint32_t)eChannel);
			if ( eStatus == NETDEV_STATUS_OK)
			{
				eStatus = WizeNet_Ioctl(&sNetDev, NETDEV_CTL_SET_DWLINK_MOD, (uint32_t)eMod);
			}
			xSemaphoreGive(sNetDev.hLock);
		}
	}
	return eStatus;
}

/*!
 * @brief This function set/get the network device ctl
 *
 * @param [in] eCtl CTL to set
 * @param [in] args extra argument
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Ioctl(uint32_t eCtl, uint32_t args)
{
	int32_t eStatus = NET_STATUS_BUSY;
	// check if caller own the NetMgr mutex
	if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
	{
		// try to acquire the Net Dev
		if ( xSemaphoreTake( sNetDev.hLock, NET_DEV_ACQUIRE_TIMEOUT()) )
		{
			eStatus = WizeNet_Ioctl(&sNetDev, eCtl, args);
			xSemaphoreGive(sNetDev.hLock);
		}
	}
	return eStatus;
}

/*!
 * @brief This function send the given message
 *
 * @param[in] pxNetMsg   Pointer to the message to send
 * @param[in] u32TimeOut Timeout in millisecond
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Send(net_msg_t *pxNetMsg, uint32_t u32TimeOut)
{
	int32_t eStatus;
	uint32_t tmoCoarse;
	int16_t tmoFine;

	tmoCoarse = u32TimeOut/1000;
	tmoFine = u32TimeOut - tmoCoarse*1000;

	eStatus = NET_STATUS_ERROR;
	if ( pxNetMsg && pxNetMsg->pData)
	{
		if ( pxNetMsg->u8Type >= APP_TYPE_NB )
		{
			LOG_ERR("APP type UNKNOWN\n");
			return eStatus;
		}

		eStatus = NET_STATUS_BUSY;
		// check if caller own the NetMgr mutex
		if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
		{
			// try to acquire the Net Dev
			if ( xSemaphoreTake( sNetDev.hLock, NET_DEV_ACQUIRE_TIMEOUT()) )
			{
				sWizeCtx.pBuffDesc = (void*)pxNetMsg;
				sWizeCtx.u8Type = pxNetMsg->u8Type;
				sWizeCtx.eListenType = 0;

				// try to send
				eStatus = _net_mgr_send_with_retry_(&sNetDev, pxNetMsg, sWizeCtx.u8TransRetries);
				if ( eStatus == NETDEV_STATUS_OK )
				{
					if ( TimeEvt_TimerStart(&sWizeCtx.sTimeOut, tmoCoarse, tmoFine,	(uint32_t)NETDEV_EVT_TIMEOUT ))
					{
						_net_mgr_try_abort_(&sNetDev);
						eStatus = NET_STATUS_ERROR;
					}
				}
			}
		}
	}
	return eStatus;
}

/*!
 * @brief This function listen for the given message
 *
 * @param[in] pxNetMsg    Pointer to the message to listen. The net_msg_t::u8Type
 *                        field select the filtered message).
 * @param[in] u32TimeOut  Timeout in millisecond
 * @param[in] eListenType Listen type define the relationship between the
 *                        timeout and the receipting message.
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_Listen(net_msg_t *pxNetMsg, uint32_t u32TimeOut, net_listen_type_e eListenType)
{
	int32_t eStatus;
	uint32_t tmoCoarse;
	int16_t tmoFine;

	tmoCoarse = u32TimeOut/1000;
	tmoFine = u32TimeOut - tmoCoarse*1000;

	eStatus = NET_STATUS_ERROR;
	if ( pxNetMsg && pxNetMsg->pData)
	{
		if ( pxNetMsg->u8Type >= APP_TYPE_NB )
		{
			LOG_ERR("APP type UNKNOWN\n");
			return eStatus;
		}

		eStatus = NET_STATUS_BUSY;
		// check if caller own the NetMgr mutex
		if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
		{
			// try to acquire the Net Dev
			if ( xSemaphoreTake( sNetDev.hLock, NET_DEV_ACQUIRE_TIMEOUT())  )
			{
				sWizeCtx.pBuffDesc = (void*)pxNetMsg;
				sWizeCtx.u8Type = pxNetMsg->u8Type;
				sWizeCtx.eListenType = eListenType;

				// try to listen
				eStatus = _net_mgr_listen_with_retry_(&sNetDev, sWizeCtx.u8RecvRetries);
				if ( eStatus == NETDEV_STATUS_OK )
				{
					if ( TimeEvt_TimerStart(&sWizeCtx.sTimeOut, tmoCoarse, tmoFine,	(uint32_t)NETDEV_EVT_TIMEOUT ))
					{
						_net_mgr_try_abort_(&sNetDev);
						eStatus = NET_STATUS_ERROR;
					}
				}
			}
		}
	}
	return eStatus;
}

/*!
 * @brief This function notify that previous listen fnet_msg_t buffer is no more pending.
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
int32_t NetMgr_ListenReady(void)
{
	int32_t eStatus;
	eStatus = NET_STATUS_OK;
	// check if caller own the NetMgr mutex
	if (sWizeCtx.hCaller == xTaskGetCurrentTaskHandle( ) )
	{
		xTaskNotify(sWizeCtx.hTask, _NET_REARM_LISTEN_, eSetBits);
	}
	else
	{
		eStatus = NET_STATUS_BUSY;
	}
	return eStatus;
}

/******************************************************************************/
/*!
 * @static
 * @brief This is the main task function, as FSM that treat events from
 * net/phy and to/from other tasks
 *
 * @param [in] argument (not used)
 *
 * @return      None
 */
static void _net_mgr_main_(void const * argument)
{
	uint32_t u32Evt;
	uint32_t u32BackEvt;
	uint8_t bAbort = 0;
	uint8_t bError = 0;

	while (1)
	{
		u32BackEvt = NET_EVENT_NONE;
		bError = 0;
		bAbort = 0;

		// waiting for event : timeout ...
		if ( xTaskNotifyWait(0, ULONG_MAX, &u32Evt, NET_MGR_EVT_TIMEOUT()) == 1 )
		{
			// Treat NetDev state
			switch (sNetDev.eState)
			{
				case NETDEV_STATE_ERROR:
				{
					LOG_DBG("NET State ERROR\n");
					bError = 1;
					break;
				}
				case NETDEV_STATE_UNKWON:
				{
					// Maybe it's not initialized
					LOG_DBG("NET State UNKOWN\n");
					break;
				}
				// NetDev is busy, so wait for an event
				case NETDEV_STATE_BUSY:
				case NETDEV_STATE_IDLE:
				{
					u32BackEvt = _net_mgr_fsm_(&sNetDev, u32Evt);

					if( u32BackEvt & NET_EVENT_TIMEOUT )
					{
						bAbort = 1;
					}
					if( u32BackEvt & NET_EVENT_ERROR )
					{
						bError = 1;
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}
		// else, do nothing

		if (bError)
		{
			if (_net_mgr_error_(&sNetDev))
			{
				bAbort = 1;
			}
		}

		if (bAbort)
		{
			// Abort the current request
			if ( _net_mgr_try_abort_(&sNetDev) )
			{
				// ulBckFlag = NET_EVT_FATAL;
			}
			WizeNet_Ioctl(&sNetDev, NETDEV_CTL_PHY_CMD, PHY_CTL_CMD_SLEEP);
		}

		// send back notify to the caller
		_net_mgr_notify_caller_(u32BackEvt);

		if (bError || bAbort)
		{
			xSemaphoreGive(sNetDev.hLock);
		}
	}
}

/*!
 * @static
 * @brief  ISR Callback function to notify an event occurs
 *
 * @param [in]  evt Notified event
 *
 * @return      None
 */
static void _net_mgr_evtCb_(uint32_t evt)
{
	BaseType_t xHigherPriorityTaskWoken;
	xTaskNotifyFromISR(sWizeCtx.hTask, evt, eSetBits, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/*!
 * @static
 * @brief  Notify the caller an event occurs
 *
 * @param [in]  evt Notified event
 *
 * @return      None
 */
static inline void _net_mgr_notify_caller_(uint32_t evt)
{
	if (sWizeCtx.hCaller)
	{
		if (evt != NET_EVENT_NONE)
		{
			xTaskNotify(sWizeCtx.hCaller, evt, eSetBits);
		}
	}
}

/******************************************************************************/

/*!
 * @static
 * @brief Internal fsm
 *
 * @param[in] pNetDev Pointer to NetDev device
 * @param[in] u32Evt  Event to treat
 *
 * @retval
 * @li @link net_event_e::NET_EVENT_SUCCESS @endlink
 * @li @link net_event_e::NET_EVENT_ERROR @endlink
 * @li @link net_event_e::NET_EVENT_SEND_DONE @endlink
 * @li @link net_event_e::NET_EVENT_RECV_DONE @endlink
 * @li @link net_event_e::NET_EVENT_FRM_PASSED @endlink
 * @li @link net_event_e::NET_EVENT_TIMEOUT @endlink
 */
static uint32_t _net_mgr_fsm_(netdev_t *pNetDev, uint32_t u32Evt)
{
	uint32_t u32BackEvt;
	int32_t eStatus;
	net_msg_t *pxNetMsg;

	u32BackEvt = NET_EVENT_SUCCESS;
	pxNetMsg = (net_msg_t*)(sWizeCtx.pBuffDesc);

	if(u32Evt & _NET_REARM_LISTEN_)
	{
		sWizeCtx.bListenPend = 0;
	}

	if (u32Evt & NETDEV_EVT_TX_COMPLETE)
	{
		TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
		WizeNet_Ioctl(&sNetDev, NETDEV_CTL_PHY_CMD, PHY_CTL_CMD_SLEEP);
		xSemaphoreGive(sNetDev.hLock);
		u32BackEvt |= NET_EVENT_SEND_DONE;
		LOG_FRM_OUT(
				((wize_net_t*)sNetDev.pCtx)->aSendBuff,
				((wize_net_t*)sNetDev.pCtx)->aSendBuff[0]+1
				);
	}

	if(u32Evt & NETDEV_EVT_RX_STARTED)
	{
		if (sWizeCtx.eListenType == NET_LISTEN_TYPE_DETECT)
		{
			sWizeCtx.u8Detected |= 0x01;
		}
	}

	if(u32Evt & NETDEV_EVT_RX_COMPLETE)
	{
		// ensure to not overwriting net_msg_t buffer
		if ( sWizeCtx.bListenPend && sWizeCtx.eListenType == NET_LISTEN_TYPE_MANY)
		{
			// try to listen again
			eStatus = _net_mgr_listen_with_retry_(pNetDev, sWizeCtx.u8RecvRetries);
			if ( eStatus != NETDEV_STATUS_OK )
			{
				// Stop time event
				TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
				// request failed, cancel the session
				u32BackEvt = NET_EVENT_ERROR;
			}
		}
		else
		{
			eStatus = WizeNet_Recv(pNetDev, pxNetMsg );
			if ( eStatus == NETDEV_STATUS_OK)
			{
				if (pxNetMsg->u8Type == sWizeCtx.u8Type)
				{
					if (sWizeCtx.eListenType == NET_LISTEN_TYPE_MANY)
					{
						eStatus = _net_mgr_listen_with_retry_(pNetDev, sWizeCtx.u8RecvRetries);
						if ( eStatus != NETDEV_STATUS_OK )
						{
							// Stop time event
							TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
							// request failed, cancel the session
							u32BackEvt = NET_EVENT_ERROR;
						}
						else
						{
							sWizeCtx.bListenPend = 1;
						}
					}
					else
					{
						// Stop time event
						TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
						xSemaphoreGive(sNetDev.hLock);
					}
					u32BackEvt |= NET_EVENT_RECV_DONE;
					LOG_FRM_IN(
							((wize_net_t*)sNetDev.pCtx)->aRecvBuff,
							((wize_net_t*)sNetDev.pCtx)->aRecvBuff[0]+1
							);
				}
				else // received message doesn't match
				{
					// try to listen again
					eStatus = _net_mgr_listen_with_retry_(pNetDev, sWizeCtx.u8RecvRetries);
					if ( eStatus != NETDEV_STATUS_OK )
					{
						// Stop time event
						TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
						// request failed, cancel the session
						u32BackEvt = NET_EVENT_ERROR;
					}
					u32BackEvt |= NET_EVENT_FRM_PASSED;
				}
			}
			else
			{
				if (_net_mgr_error_(pNetDev))
				{
					//abort
					// Stop time event
					TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
					// request failed, cancel the session
					u32BackEvt = NET_EVENT_TIMEOUT;
				}
				else
				{
					// try to listen again
					eStatus = _net_mgr_listen_with_retry_(pNetDev, sWizeCtx.u8RecvRetries);
					if ( eStatus != NETDEV_STATUS_OK )
					{
						// Stop time event
						TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
						// request failed, cancel the session
						u32BackEvt = NET_EVENT_ERROR;
					}
				}
			}
			// FIXME
			/*
			if (try_listen_again)
			{
				eStatus = _net_mgr_listen_with_retry_(pNetDev, sWizeCtx.u8RecvRetries);
			}

			// stop_error
			if ( eStatus != NETDEV_STATUS_OK )
			{
				// Stop time event
				TimeEvt_TimerStop(&sWizeCtx.sTimeOut);
				// request failed, cancel the session
				u32BackEvt = NET_EVENT_ERROR;
			}
			*/
		}
		sWizeCtx.u8Detected = 0;
	}

	if (u32Evt & NETDEV_EVT_TIMEOUT)
	{
		if (sWizeCtx.eListenType == NET_LISTEN_TYPE_DETECT)
		{
			// check if we have already expand the timeout or detected starting frame
			if ( !(sWizeCtx.u8Detected) || (sWizeCtx.u8Detected & _NET_EXPAND_TMO_MSK_) )
			{
				sWizeCtx.u8Detected = 0;
				u32BackEvt |= NET_EVENT_TIMEOUT;
			}
			else
			{
				// set expand timeout
				sWizeCtx.u8Detected = _NET_EXPAND_TMO_MSK_;
				if ( TimeEvt_TimerStart(
						&sWizeCtx.sTimeOut,
						0, sWizeCtx.i16ExpandTmo,
						(uint32_t)NETDEV_EVT_TIMEOUT
						))
				{
					u32BackEvt = NET_EVENT_ERROR;
				}
			}
		}
		else
		{
			u32BackEvt |= NET_EVENT_TIMEOUT;
		}

		/*
		if( u32BackEvt & (NET_EVENT_ERROR | NET_EVENT_TIMEOUT) )
		{
			_net_mgr_try_abort_(pNetDev);
			WizeNet_Sleep(pNetDev);
		}
		*/
	}

	return u32BackEvt;
}

/*!
 * @static
 * @brief Internal function to send the given message with retry
 *
 * @param[in] pNetDev  Pointer to device to send the message
 * @param[in] pxNetMsg Pointer to the message to send
 * @param[in] u8Retry  Number of retry
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
static int32_t _net_mgr_send_with_retry_(netdev_t *pNetDev, net_msg_t *pxNetMsg, uint8_t u8Retry)
{
	int32_t eStatus = NETDEV_STATUS_OK;
	do {
		// try to send
		eStatus = WizeNet_Send(pNetDev, pxNetMsg);
		if ( eStatus == NETDEV_STATUS_OK )
		{
			break; // everything is fine
		}
		else if ( eStatus == NETDEV_STATUS_ERROR)
		{
			// get error
			if ( _net_mgr_error_(pNetDev) )
			{
				// try to abort the current Net Device Action
				_net_mgr_try_abort_(pNetDev);
			}
		}
		// else // NET_STATUS_BUSY
		if (!u8Retry)
		{
			break;
		}
		u8Retry--;
	} while (1);
	return eStatus;
}

/*!
 * @static
 * @brief Internal function to listen for message with retry
 *
 * @param[in] pNetDev  Pointer to device to listen the message
 * @param[in] u8Retry  Number of retry
 *
 * @retval
 * @li @link net_status_e::NET_STATUS_OK @endlink
 * @li @link net_status_e::NET_STATUS_ERROR @endlink
 * @li @link net_status_e::NET_STATUS_BUSY @endlink
 */
static int32_t _net_mgr_listen_with_retry_(netdev_t *pNetDev, uint8_t u8Retry)
{
	int32_t eStatus;
	do {
		// try to listen
		eStatus = WizeNet_Listen(pNetDev);
		if ( eStatus == NETDEV_STATUS_OK )
		{
			break; // everything is fine
		}
		else if ( eStatus == NETDEV_STATUS_ERROR)
		{
			// get error
			if ( _net_mgr_error_(pNetDev) )
			{
				// try to abort the current Net Device Action
				_net_mgr_try_abort_(pNetDev);
			}
		}
		// else // NET_STATUS_BUSY
		if (!u8Retry)
		{
			break;
		}
		u8Retry--;
	} while (1);
	return eStatus;
}

/*!
 * @static
 * @brief  Internal function to get and clear errors
 *
 * @retval  0 Success
 * @retval  1 Phy abort is required
 */
static int32_t _net_mgr_error_(netdev_t *pNetDev)
{
	int32_t eRet = 0;
	const char *pErrStr = NULL;
	// Get the current error
	WizeNet_Ioctl(pNetDev, NETDEV_CTL_GET_STR_ERR, (uint32_t)(&pErrStr));
	switch(pNetDev->eErrType)
	{
		// error on protocol
		case NETDEV_ERROR_PROTO:
			// show error
			if ( WizeNet_Ioctl(pNetDev, NETDEV_CTL_GET_ERR, 0) >= PROTO_FRM_WRN )
			{
				LOG_WRN("PROTO : %s\n", pErrStr);
			}
			else
			{
				LOG_ERR("PROTO : %s\n", pErrStr);
			}
			break;
		// error on phy
		case NETDEV_ERROR_PHY:
			// show error
			LOG_ERR("PHY : %s\n", pErrStr);
			// Abort the current request
			eRet = 1;
			break;
		default:
			break;
	};
	WizeNet_Ioctl(pNetDev, NETDEV_CTL_CLR_ERR, 0);
	return eRet;
}

/*!
 * @static
 * @brief  Internal function to abort and clean the current net/phy state
 *
 * @retval  0 Success
 * @retval  1 Fatal : unable to Abort nor Reset the Phy device
 */
static int32_t _net_mgr_try_abort_(netdev_t *pNetDev)
{
	int32_t eRet = 0;
	// Abort the current request
	if ( WizeNet_Ioctl(pNetDev, NETDEV_CTL_PHY_CMD, PHY_CTL_CMD_READY) != NETDEV_STATUS_OK )
	{
		// Fatal, try to restart the module
		LOG_ERR("NET Abort\n");
		_net_mgr_error_(pNetDev);
		if ( WizeNet_Ioctl(pNetDev, NETDEV_CTL_PHY_CMD, PHY_CTL_CMD_RESET) != NETDEV_STATUS_OK )
		{
			LOG_ERR("NET Reset\n");
			//assert(0);
			eRet = 1; // should never reach it
		}
	}
	return eRet;
}

/*! @} */
/*! @} */
/*! @} */

#ifdef __cplusplus
}
#endif
