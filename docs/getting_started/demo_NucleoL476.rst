
Demo application
================

- This application is provide as a simple demonstrator of OpenWize stack targeting an ST Nucleo-L476RG board. 

- The Nucleo-L476RG board doesn't integrate any RF device, so to overcome this, the demo application use the ``PhyFake`` device, which input/output frames over an UART peripheral.

- This demo is very simple so, there is no console or local interface to communicate with it. Instead, application periodically send DATA and INSTALL (aka. PING) message and use the provided *Logger* module to print out messages (info, warning, debug, error). 

- The *Logger* is connected to the MCU USART2 peripheral

   - Configuration : 115200 bps, 8 bits data, no parity, 1 stop bit.
   - This USART is mapped onto CN1 (USB-mini).

- The *PhyFake* is connected to the MCU LPUART1 peripheral :

   - Configuration : 115200 bps, 8 bits data, no parity, 1 stop bit.
   
   - This USART is mapped onto : 
   
      - RX : PC0 (CN7-38)
      - TX : PC1 (CN7-36).

- Some default parameters have been set to :

   .. list-table:: 
     :align: center
     :widths: auto 
     :header-rows: 1

     * - ID
       - Name
       - Value
     * - 0x28
       - CIPH_CURRENT_KEY
       - 0
     * - 0x18
       - EXCH_RX_DELAY
       - 1
     * - 0x19
       - EXCH_RX_LENGTH
       - 1
     * - 0x1A
       - EXCH_RESPONSE_DELAY
       - 1
     * - 0x30
       - PING_RX_DELAY
       - 1
     * - 0x31
       - PING_RX_LENGTH
       - 1


Build the application
---------------------

.. code-block:: bash

   cd Openwize
   mkdir -p _build

.. code-block:: bash

   cmake -DBUILD_CFG=Nucleo-L476/Nucleo-L476 -DCMAKE_BUILD_TYPE=Debug ../. 
   make DemoApp -j
   make install
   
The firmware files are installed in OpenWize/*_install* directory

:: 

   _install/
      └── bin
          ├── DemoApp.elf  : The "elf" file (i.e. with debug symbols)
          ├── DemoApp.bin  : Pure binary file
          ├── DemoApp.lst  : Disassembly listing
          └── DemoApp.map  : Symbols and files mapping


Load and run the firmware
-------------------------
To be able to load and run the demo application, you will need an ST Nucleo-L476
board and and USB-to-UART converter.

#. Connect your Nucleo-L476RG board to your computer
#. Connect the USB-to-UART to your computer
#. Check ST-Link probe id

   .. code-block:: bash

      cd OpenWize
      export PATH=$PATH:$(pwd)/tools/scripts/test_support
      twk_load_stlink.sh -h

   You should get something like that :

   .. code-block::

      Error! Give a probe index to be able to continue.
      Available devices :
            -------------------------------------------------------------------
                              STM32CubeProgrammer v2.2.0                  
            -------------------------------------------------------------------

      =====  DFU Interface   =====

      No STM32 device in DFU mode connected

      ===== STLink Interface =====

      -------- Connected ST-LINK Probes List --------

      ST-Link Probe 0 :
         ST-LINK SN  : 066CFF383333554157243011
         ST-LINK FW  : V2J34M25
      -----------------------------------------------

      =====  UART Interface  =====

      Total number of serial ports available: 4

      Port: ttyUSB0
      Location: /dev/ttyUSB0
      Description: FT232R USB UART
      Manufacturer: FTDI

      Port: ttyACM0
      Location: /dev/ttyACM0
      Description: STM32 STLink
      Manufacturer: STMicroelectronics

      Port: ttyS0
      Location: /dev/ttyS0
      Description: N/A
      Manufacturer: N/A
      
.........

   In this example, the probe id id 0 :
   
   :: 

      ST-Link Probe 0 :
      ST-LINK SN  : 066CFF383333554157243011
      ...


   Then, upload the firmware onto the board
   
   .. code-block:: bash

      twk_load_stlink.sh ../_install/bin/DemoApp.bin 0

   You should be able to the *Logger* messages on *ttyACM0* and Wize frames on *ttyUSB0*.


Debug the demo application
--------------------------

In the directory "OpenWize/demo/project", we provides Eclipse project files. Open
the STMCube32 IDE and *Import* as *Existing Projects into Workspace*.



.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
