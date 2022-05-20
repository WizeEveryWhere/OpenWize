
Porting guide
=============

..
   Add an application to the Nucleo L476 demo
   ------------------------------------------
   TBD 

Porting your PHY device
-----------------------

Take a look at the *PhyFake* demo device that gives a simple example.

Prepare your module
^^^^^^^^^^^^^^^^^^^

- Create a directory for your PHY device.
- Create the file *include/phy_layer_private.h*.
- Create the files *src/phy_layer.c* and *src/phy_layer_private.c*.
- Create the *CMakeLists.txt*

Then, that should gives something like :

:: 

   MyPhy
   ├── CMakeLists.txt
   ├── include
   │   └── phy_layer_private.h
   └── src
       ├── phy_layer_private.c
       └── phy_layer.c

Implement the low level driver
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In *phy_layer_private.h* and *phy_layer_private.c* integrate all requirement (function, defines, typedef, ...) to drive at low level your device. Its content is very dependent of your device behavior.   

Implement the phy driver
^^^^^^^^^^^^^^^^^^^^^^^^

The *phy_layer.c* is intended to implement the OpenWize phy interface. 

Phy interface
"""""""""""""

Include the phy interafce definition from *phy_itf.h*.
Define the interface implementation :

.. code-block:: c

   static const phy_if_t _phy_if = {
      .pfInit          = _my_init_,
      .pfUnInit        = _my_uninit_,
      .pfTx            = _my_TX_,
      .pfRx            = _my_RX_,
      .pfNoise         = _my_noise_,
      .pfSetSend       = _my_set_send_,
      .pfGetRecv       = _my_get_recv_,
      .pfIoctl         = _my_ioctl_
   };

Implement the interface functions :

- **_my_init_**     : Driver inititialization function.

   .. code-block:: c
      
      static int32_t _my_init_(phydev_t *pPhydev);
 
   This function must initialize : 
   
   - The *pPhydev* internal structure.
   - The bsp bus communication (if any).
   - Your *my_phy_device_t* device internal structure.

- **_my_uninit_**   : Driver un-inititialization function.

   .. code-block:: c
      
      static int32_t _my_uninit_(phydev_t *pPhydev)
   
   This function must uninitialize : 
   
   - The bsp bus communication (if any).

- **_my_TX_**       : Do the transmit action 

   .. code-block:: c
         
         static int32_t _my_TX_(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);
   
   This function must :
   
   - Take into account the channel and modulation changes.
   - Call low level sending function.  

- **_my_RX_**       : Do the listen action.

   .. code-block:: c

      static int32_t _my_RX_(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);

   This function must :
   
   - Take into account the channel and modulation changes.
   - Call low level listening function.

- **_my_noise_**    : Measure the noise.

   .. code-block:: c

      static int32_t _my_noise_(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);

   This function is optional. 

- **_my_set_send_** : Transfert the frame into the device internal transmiting buffer

   .. code-block:: c

      static int32_t _my_set_send_(phydev_t *pPhydev, uint8_t *pBuf, uint8_t u8Len);
   
   This function must : 
   
   - Transfert the content of pBuf into the device internal buffer.

- **_my_get_recv_** : Get the frame from the device internal receiving buffer.

   .. code-block:: c

      static int32_t _my_get_recv_(phydev_t *pPhydev, uint8_t *pBuf, uint8_t *u8Len);

   This function must : 
   
   - Get the content the device internal buffer and copy it into the pBuf.

- **_my_ioctl_**    : Get/Set the device configuration 

   .. code-block:: c

      static int32_t _my_ioctl_(phydev_t *pPhydev, uint32_t eCtl, uint32_t args);
   
   Phy IO control is given by *eCtl* while *args* is the value to get/set. See phy_ctl_e for details information.  
   

Setup function
""""""""""""""

This function is intended to prepare the Phy device with constant configuration.

.. code-block:: c
   
   int32_t Phy_MyPhy_setup(phydev_t *pPhydev, my_phy_device_t *pCtx)


The minimal implementation is the following :

.. code-block:: c

   int32_t Phy_MyPhy_setup(phydev_t *pPhydev, my_phy_device_t *pCtx)
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

Interrupt handler
"""""""""""""""""

This handler must be called when an interrupt occurs from your device and should look like the following :

.. code-block:: c

   static void _my_frame_it_(void *p_CbParam, void *p_Arg)
   {
      uint32_t eEvt = PHYDEV_EVT_NONE;
      phydev_t *pPhydev = (phydev_t *) p_CbParam;
      my_phy_device_t *pDevice;
      uint32_t u32IrqStatus = MY_PHY_EVT_NONE;
      
      if (pPhydev)
      {
         pDevice = pPhydev->pCxt;
         // ...do something with private structure, if required
         // Get irq status 
         // ...from passing argument 
         // u32IrqStatus = ((uint32_t)p_Arg);
         // ...or from calling low level function
         // u32IrqStatus = MyPhy_GetIrq(...);
      }
      
      if (u32IrqStatus == MY_PHY_EVT_RX_STARTED)
      {
         eEvt = PHYDEV_EVT_RX_STARTED;
         // ...implement some action on this event, if any
      }
      if (u32IrqStatus == MY_PHY_EVT_TX_CPLT)
      {
         eEvt = PHYDEV_EVT_TX_COMPLETE;
         // ...implement some action on this event, if any
      }
      if (u32IrqStatus == MY_PHY_EVT_RX_CPLT)
      {
         eEvt = PHYDEV_EVT_RX_COMPLETE;
         // ...implement some action on this event, if any
      }
      
      // event notification
      if( (eEvt != PHYDEV_EVT_NONE) && pPhydev->pfEvtCb ) 
      {
         // Notify the higher level
         pPhydev->pfEvtCb(pPhydev->pCbParam, eEvt);
      }
   }

Porting to a new board
----------------------

TBD

..
   Requirement : 
   
   - board : TBD
   - bsp : TBD
   - device : TBD
   - application : TBD

Compiler
--------
TBD
