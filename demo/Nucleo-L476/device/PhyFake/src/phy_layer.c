/**
  * @file: phy_layer.c
  * @brief: This file implement the phy device driver
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
  * 1.0.0 : 2020/05/15[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Device
 * @{
 * @ingroup Wize
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <bsp.h>
#include <platform.h>

#include "phy_layer_private.h"

const char * const phyfake_error_msgs[] =
{
    [FAKEUART_ERR_NONE]      = "PhyFake ERR_NONE",
	[FAKEUART_FAILURE]       = "PhyFake FAILURE",
	[FAKEUART_BUSY]          = "PhyFake BUSY",
	[FAKEUART_TIMEOUT]       = "PhyFake TIMEOUT",
	[FAKEUART_INVALID_PARAM] = "PhyFake INVALID_PARAM",
};

/*!
 * @brief This convenient table hold the human channel representation
 */
const char * const aChanStr[PHY_NB_CH] = {
    [PHY_CH100] = "100",
    [PHY_CH110] = "110",
    [PHY_CH120] = "120",
    [PHY_CH130] = "130",
    [PHY_CH140] = "140",
    [PHY_CH150] = "150",
};

/*!
 * @brief This convenient table hold the human modulation representation
 */
const char * const aModulationStr[PHY_NB_MOD] = {
    [PHY_WM2400] = "WM2400",
    [PHY_WM4800] = "WM4800",
    [PHY_WM6400] = "WM6400",
};

/*!
 * @brief This convenient table hold the human test mode representation
 */
const char * const aTestModeStr[PHY_NB_TST_MODE] = {
    [PHY_TST_MODE_NONE]   = "Disable",
    [PHY_TST_MODE_RX]     = "RX",
    [PHY_TST_MODE_PER_RX] = "RX PER",
	[PHY_TST_MODE_TX]     = "TX",
};

// Private function (mapped to interface)
static int32_t _init(phydev_t *pPhydev);
static int32_t _uninit(phydev_t *pPhydev);

static int32_t _do_TX(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);
static int32_t _do_RX(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);
static int32_t _do_CCA(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);

static int32_t _set_send(phydev_t *pPhydev, uint8_t *pBuf, uint8_t u8Len);
static int32_t _get_recv(phydev_t *pPhydev, uint8_t *pBuf, uint8_t *u8Len);

static int32_t _ioctl(phydev_t *pPhydev, uint32_t eCtl, uint32_t args);

/*!
 * @brief This structure hold the Phy device interface
 */
static const phy_if_t _phy_if = {
    .pfInit          = _init,
    .pfUnInit        = _uninit,

	.pfTx            = _do_TX,
    .pfRx            = _do_RX,
	.pfNoise         = _do_CCA,

	.pfSetSend       = _set_send,
	.pfGetRecv       = _get_recv,

	.pfIoctl         = _ioctl
};
/*!
 * @brief  This function prepare the Phy device with constant configuration
 *
 * @param [in]  pPhydev Pointer
 *
 * @return      Status
 * - PHY_STATUS_OK     Function has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
int32_t Phy_PhyFake_setup(phydev_t *pPhydev, fakeuart_device_t *pCtx)
{
	int32_t i32Ret = PHY_STATUS_ERROR;
    if (pPhydev && pCtx)
    {
        pPhydev->pIf = &_phy_if;
        pPhydev->pCxt = pCtx;
       	i32Ret = PHY_STATUS_OK;
    }
    return i32Ret;
}

/******************************************************************************/
// Internal private function
static int32_t _do_cmd(phydev_t *pPhydev, uint8_t eCmd);
static void _frame_it(void *p_CbParam, void *p_Arg);

/*!
 * @static
 * @brief  This function initialize the Phy device
 *
 * @param [in]  pPhydev Pointer on the Phy device instance
 *
 * @return      Status
 * - PHY_STATUS_OK     Function has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _init(phydev_t *pPhydev)
{
    int32_t i32Ret = PHY_STATUS_ERROR;
    fakeuart_device_t *pDevice;
    if(pPhydev)
    {
    	pDevice = pPhydev->pCxt;
		// set default parameters
		pPhydev->i16TxFreqOffset = DEFAULT_TX_FREQ_OFFSET;
		pPhydev->eModulation = DEFAULT_MOD;
		pPhydev->eTxPower = DEFAULT_TX_POWER;
		pPhydev->eChannel = DEFAULT_CH;
		pPhydev->pfEvtCb = NULL;
		pPhydev->pCbParam = NULL;
		pPhydev->bPreSyncOn = 0;
		pPhydev->bCrcOn = 0;
		pPhydev->eTestMode = PHY_TST_MODE_NONE;

		pDevice->eState = IDLE_STATE;

		BSP_Uart_SetCallback (UART_ID_PHY, _frame_it, pPhydev);
		if ( BSP_Uart_Init( UART_ID_PHY, '\r', UART_MODE_EOB, 0) == DEV_SUCCESS)
		{
			pDevice->eState = INITIALIZED_STATE;
			i32Ret = PHY_STATUS_OK;
		}
    }
    return i32Ret;
}

/*!
 * @static
 * @brief  This function un-initialize the Phy device. (Power OFF and Reset released)
 *
 * @param [in]  pPhydev Pointer on the Phy device instance
 *
 * @return      Status
 * - PHY_STATUS_OK     Function has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _uninit(phydev_t *pPhydev)
{
    int32_t i32Ret = PHY_STATUS_ERROR;
    fakeuart_device_t *pDevice;
    if(pPhydev)
    {
    	pDevice = pPhydev->pCxt;
    	pDevice->eState = IDLE_STATE;
		i32Ret = PHY_STATUS_OK;
    }
    return i32Ret;
}

/******************************************************************************/
/******************************************************************************/

/*!
 * @brief  This is the main FSM.
 *
 * @details This function ensure that transition between state are respected.
 * ... :
 * - Wake-up, (re)configuration,
 *
 * @param [in] pPhydev    Pointer on the Phy device instance
 * @param [in] eCmd       Requested command to execute
 *
 * @return             Status
 * - PHY_STATUS_OK     Requested command has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 */
static int32_t _do_cmd(phydev_t *pPhydev, uint8_t eCmd)
{
	int32_t eStatus = PHY_STATUS_OK;
	fakeuart_device_t *pDevice = pPhydev->pCxt;
	(void)pDevice;
	if ( eCmd < PHY_CTL_CMD_READY)
	{
		pPhydev->bCrcOn = 0;
		pPhydev->bPreSyncOn = 0;

		pPhydev->u16_Noise = 0;
		pPhydev->u16_Rssi  = 0;
		pPhydev->u16_Ferr  = 0;
		pPhydev->eTestMode = PHY_TST_MODE_NONE;

		switch(eCmd)
		{
			case PHY_CTL_CMD_PWR_OFF:
				break;
			case PHY_CTL_CMD_PWR_ON:
			case PHY_CTL_CMD_RESET:
			default:
				break;
		}
	}
    else
    {
		switch (eCmd)
		{
			case PHY_CTL_CMD_READY:
				break;
			case PHY_CTL_CMD_SLEEP:
				break;
			default:
				break;
		}
    }
    return eStatus;
}

/*!
 * @brief  Interruption handler to treat the frame event
 *
 * @param [in] p_CbParam Pointer on call-back parameter
 * @param [in] p_Arg     Pointer on call-back argument
 *
 * @return None
 */
static void _frame_it(void *p_CbParam, void *p_Arg)
{
	uint32_t eEvt = PHYDEV_EVT_NONE;
	phydev_t *pPhydev = (phydev_t *) p_CbParam;
	fakeuart_device_t *pDevice;
    uint32_t u32IrqStatus = UART_EVT_NONE;

    if (pPhydev)
    {
    	pDevice = pPhydev->pCxt;
    	u32IrqStatus = ((uint32_t)p_Arg);
    }

	if (u32IrqStatus == UART_EVT_RX_HCPLT)
	{
		eEvt = PHYDEV_EVT_RX_STARTED;
		//if (pDevice->NAD == pPhydev->eChannel) { }
	}
	if (u32IrqStatus == UART_EVT_TX_CPLT)
	{
		eEvt = PHYDEV_EVT_TX_COMPLETE;
		pDevice->eState &= ~(TRANSMITTING_STATE);
	}
	if (u32IrqStatus == UART_EVT_RX_CPLT)
	{
		eEvt = PHYDEV_EVT_RX_COMPLETE;
		pDevice->eState &= ~(RECEIVING_STATE);
	}

    // event notification
    if( (eEvt != PHYDEV_EVT_NONE) && pPhydev->pfEvtCb ) {
		pPhydev->pfEvtCb(pPhydev->pCbParam, eEvt);
	}
}

/******************************************************************************/
/******************************************************************************/

/*!
 * @static
 * @brief  This function execute a TX sequence
 *
 * @param [in]  pPhydev     Pointer on the Phy device instance
 * @param [in]  eChannel    Channel use to TX
 * @param [in]  eModulation Modulation use to TX
 *
 * @return      Status
 * - PHY_STATUS_OK     Requested command has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _do_TX(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation)
{
    int32_t i32Ret = PHY_STATUS_ERROR;
	fakeuart_device_t *pDevice = pPhydev->pCxt;
	if (pDevice)
	{
		pPhydev->eChannel = eChannel;
		pPhydev->eModulation = eModulation;

		pDevice->eError = BSP_Uart_Transmit(UART_ID_PHY, pDevice->pBuf, pDevice->u16Len);
		if ( pDevice->eError == DEV_SUCCESS)
		{
			pDevice->eState = TRANSMITTING_STATE;
			i32Ret = PHY_STATUS_OK;
		}
	}
    return i32Ret;
}

/*!
 * @static
 * @brief  This function execute an RX sequence
 *
 * @param [in]  pPhydev     Pointer on the Phy device instance
 * @param [in]  eChannel    Channel use to RX
 * @param [in]  eModulation Modulation use to RX
 *
 * @return      Status
 * - PHY_STATUS_OK     Requested command has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _do_RX(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation)
{
    int32_t i32Ret = PHY_STATUS_ERROR;
	fakeuart_device_t *pDevice = pPhydev->pCxt;
	if (pDevice)
	{
		pPhydev->eChannel = eChannel;
		pPhydev->eModulation = eModulation;

		pDevice->eError = BSP_Uart_Receive(UART_ID_COM, pDevice->pBuf, BUF_SZ);
		if ( pDevice->eError == DEV_SUCCESS)
		{
			pDevice->eState = RECEIVING_STATE;
			i32Ret = PHY_STATUS_OK;
		}
	}
    return i32Ret;
}

/*!
 * @static
 * @brief  This function execute a Noise Measurement sequence
 *
 * @param [in]  pPhydev     Pointer on the Phy device instance
 * @param [in]  eChannel    Channel on which the Noise must be measured
 * @param [in]  eModulation Modulation on which the Noise must be measured
 *
 * @return      Status
 * - PHY_STATUS_OK     Requested command has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _do_CCA(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation)
{
    int32_t i32Ret = PHY_STATUS_OK;

    return i32Ret;
}

/*!
 * @static
 * @brief  This function set the packet to send
 *
 * @param [in]  pPhydev Pointer on the Phy device instance
 * @param [in]  pBuf    Pointer to get data to send
 * @param [in]  u8Len   Reference on the data length to send
 *
 * @return      Status
 * - PHY_STATUS_OK     Requested command has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _set_send(phydev_t *pPhydev, uint8_t *pBuf, uint8_t u8Len)
{
	int32_t i32Ret = PHY_STATUS_ERROR;
	fakeuart_device_t *pDevice = pPhydev->pCxt;
    if((!pBuf) || (!u8Len) || (!pDevice) )
    {
		pDevice->eError = FAKEUART_INVALID_PARAM;
    }
    else
    {
    	uint8_t i;
    	uint16_t *p = (uint16_t*)(pDevice->pBuf);

    	*p++ = __htons( hex2ascii(u8Len) );
    	for (i = 0; i < u8Len; i++)
    	{
    		*p++ = __htons( hex2ascii(pBuf[i]) );
    	}
		pDevice->u16Len = (u8Len +1) << 1;
		pDevice->pBuf[pDevice->u16Len] = '\r';
		pDevice->u16Len++;

		i32Ret = PHY_STATUS_OK;
    }
    return i32Ret;
}

/*!
 * @static
 * @brief  This function get the received packet
 *
 * @param [in]  pPhydev Pointer on the Phy device instance
 * @param [in]  pBuf    Pointer on buffer to get received data
 * @param [in]  u8Len   Reference on received number of bytes
 *
 * @return      Status
 * - PHY_STATUS_OK     Requested command has been successfully executed
 * - PHY_STATUS_BUSY   The device is busy (transmitting or receiving)
 * - PHY_STATUS_ERROR  Enable to communicate with the device (communication or state failure)
 *
 */
static int32_t _get_recv(phydev_t *pPhydev, uint8_t *pBuf, uint8_t* u8Len)
{
	int32_t i32Ret = PHY_STATUS_ERROR;
	fakeuart_device_t *pDevice = pPhydev->pCxt;
    if((!pBuf) || (!u8Len) || (!pDevice) )
    {
		pDevice->eError = FAKEUART_INVALID_PARAM;
    }
    else
    {
    	uint8_t i;
    	uint16_t *p = (uint16_t*)(pDevice->pBuf);
    	*u8Len = ascii2hex( __ntohs( *p++ ) );
    	for (i = 0; i < *u8Len; i++)
    	{
    		pBuf[i] = ascii2hex( __ntohs( *p++ ) );
    	}
    	pDevice->u16Len = (*u8Len + 1) << 1 ;
    	pDevice->u16Len++;
		i32Ret = PHY_STATUS_OK;
    }

    return i32Ret;
}

/******************************************************************************/
/*!
 * @static
 * @brief  This function Get/Set internal configuration variable
 *
 * @param [in]     pPhydev Pointer on the Phy device instance
 * @param [in]     eCtl    Id of configuration variable to get/set (see phy_ctl_e)
 * @param [in/out] args    scalar or pointer that hold the value to set/get
 *
 * @retval
 * @li @link phy_status_e::PHY_STATUS_OK @endlink
 * @li @link phy_status_e::PHY_STATUS_ERROR @endlink
 * @li @link phy_status_e::PHY_STATUS_BUSY @endlink
 */
static int32_t _ioctl(phydev_t *pPhydev, uint32_t eCtl, uint32_t args)
{
	int32_t i32Ret = PHY_STATUS_OK;
	fakeuart_device_t *pDevice = pPhydev->pCxt;
	if(eCtl > PHY_CTL_CMD)
	{
		i32Ret = _do_cmd(pPhydev, eCtl);
	}
	else
	{
		if(eCtl == PHY_CTL_GET_STR_ERR)
		{
			*((char*)args) = phyfake_error_msgs[pDevice->eError];
			pDevice->eError = FAKEUART_ERR_NONE;
		}
		else {
			switch(eCtl)
			{
				case PHY_CTL_SET_PA:
					// TODO :
					break;
				case PHY_CTL_SET_TX_FREQ_OFF:
					pPhydev->i16TxFreqOffset = (int16_t)args;
					break;
				case PHY_CTL_SET_TX_POWER:
					if ( (phy_power_e)args != pPhydev->eTxPower)
					{
						pPhydev->eTxPower = (phy_power_e)args;
						// run-time configure TX power is required
						// TODO :
					}
					break;
				case PHY_CTL_GET_TX_FREQ_OFF:
					*(uint8_t*)args = pPhydev->i16TxFreqOffset;
					break;
				case PHY_CTL_GET_TX_POWER:
					*(uint8_t*)args = pPhydev->eTxPower;
					break;
				case PHY_CTL_GET_FREQ_ERR:
					// TODO :
					//*(float*)args = PHY_CONV_AfcFreqErrToFloat( pPhydev->u16_Ferr );
					*(float*)args = 0;
					break;
				case PHY_CTL_GET_RSSI:
					// TODO :
					//*(uint8_t*)args = PHY_CONV_Signed11ToRssi( pPhydev->u16_Rssi );
					*(uint8_t*)args = 0;
					break;
				case PHY_CTL_GET_NOISE:
					// TODO :
					//*(uint8_t*)args = PHY_CONV_Signed11ToRssi( pPhydev->u16_Noise );
					*(uint8_t*)args = 0;
					break;
				case PHY_CTL_GET_ERR:
					// TODO :
					*(uint8_t*)args = pDevice->eError;
					break;
				default:
					break;
			}
		}
	}
	return i32Ret;
}

#ifdef __cplusplus
}
#endif

/*! @} */
/*! @} */
/*! @} */
