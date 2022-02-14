.. sectnum::

**********************
Advanced documentation
**********************

Code organization
=================

Softwares modules are organized with src/ and include/ directories, that respectively hold source and header files. In module root folder a "CMakeList.txt" file define one or more build target in order to be able to build that module. Furthermore, when is required and module is unitary "testable", a "unittest" directory containing unit tests source code and its CMakeList.txt is present. 

Typical module look like this :
:::::::::::::::::::::::::::::::

  module
  ├── CMakeLists.txt
  ├── include
  │   ├── module_private.h
  │   └── module.h
  ├── src
  │   └── module.c
  └── unittest
      ├── CMakeLists.txt
      ├── module_mock.yml
      ├── TestGrpRunModule.c
      └── TestModule.c

"sources" directory
-------------------

The "sources" directory is organized as follow :
::::::::::::::::::::::::::::::::::::::::::::::::

   sources
   ├── config.in
   ├── Samples
   │   ├── CRC_sw      
   │   ├── Crypto      
   │   ├── ImgStorage   
   │   ├── Logger      
   │   ├── Parameters   
   │   ├── ReedSolomon  
   │   └── TimeEvt
   └── WizeCore
       ├── app
       ├── mgr
       ├── net
       └── proto


There are mainly two sub-directories "Samples" and "WizeCore". "WizeCore" is the stack implementation, while "Samples" contains some required libraries by the Wize stack. The WizeCore is split in four folders, each one corresponding more or less to an abstraction layer. 

"Samples" sub directory
^^^^^^^^^^^^^^^^^^^^^^^

``CRC_sw``
   *CRC_sw* library provide function to compute and check the CRC16 (*Cyclic Redundancy Check*) as specified by `Wize Lan Protocol Specifications`_ v1.2.

``Crypto``
   *Crypto* library provide interface functions to cipher, uncipher, compute AES-CMAC and SHA256. This module rely on *tinycrypt* library located in "third-party/libraries/Tinycrypt" directory (https://github.com/intel/tinycrypt.git).

``ReedSolomon``
   *ReedSolomon* library implement Reed-Solomon code correction algorithm RS(255,223). This module is a port of the http://www.eccpage.com/rs.c

``ImgStorage``
   *ImgStorage* library provide an interface for storing downloaded firmware block on physical memory.

``Parameters``
   *Parameters* library provide interface to set and get parameters. 

``TimeEvt``
   *TimeEvt* module provide event to task at required time.  

``Logger``
   *Logger*module provide a way to log out messages (inf, warning, error, debug).
   

"WizeCore" sub directory
^^^^^^^^^^^^^^^^^^^^^^^^

``proto``
   *proto* implement all necessary to treat the link and presentation layer from the Wize protocol, that is :

   - build/extract the Link and Presentation Layers information
   - cipher/uncipher the application message
   - compute and check the authentication hash 
   - compute and check the confidentiality hash
   - compute and check the CRC
   - compute and check the Reed-Solomon code and correct the frame when possible (Download layer only)

   *proto* depends on the "CRC_sw", "Crypto" and "ReedSolomon" libraries.


``net``
   *net* implement a kind of low-level driver to : 

   - drive the protocol in *proto*
   - provide an abstract layer to higher level, with function like "send(...)" or "recev(...)".
   - provide an interface to the phy driver implementation

   *net* depends on *proto* 

``mgr``
   *mgr* implement four managers :

   - *adm_mgr*, *inst_mgr* and *dwn_mgr* manage respectively the administration, installation and download sessions. Their role is mainly to ensure the session timing correctness.
   - *net_mgr* is a kind of high level driver for the *net* module. In particular, it manage the net+phy resources in thread-safe manner and ensure the timing correctness if the "send" and "listen" windows.

   *mgr* depends on *net*, *TimeEvt*, *Logger* and *FreeRTOS*.

``app``
   *app* implement application layer of the Wize protocol and provide an API to the rest of application firmware.

   - provide API to send DATA message and execute an installation (PING/PONG) session
   - open/close administration, installation and download sessions.
   - build/extract generic administration layer (aka DATA, COMMAND, RESPONSE)
   - build/extract installation layer (aka PING/PONG)
   - deal with downloaded firmware image (store, validate)

   *app* depends on *mgr*, *TimeEvt*, *ImgStorage*, *Parameters*, *Logger* and *FreeRTOS*.


Demo directory
--------------
::

    Nucleo-L476
    ├── app : application code
    │   ├── CMakeLists.txt
    │   ├── cfg     : contains the defaults parameters configuration xml files 
    │   ├── gen     : contains the defaults parameters tables as .c and .h files
    │   ├── include : application include directory
    │   ├── src     : application source directory
    │   └── sys     : contains some code to initialization modules (RTOS, Logger, Stack...) other than the application itself
    ├── board : Contains the to initialize the board low level (peripherals, clocks) 
    │   ├── CMakeLists.txt
    │   ├── include : 
    │   ├── ld      :  
    │   ├── src     : 
    │   └── startup : 
    ├── bsp : Restricted and simple Board Support Package
    │   ├── CMakeLists.txt
    │   ├── include
    │   └── src
    ├── device 
    │   └── PhyFake : Wrapper around UART to "simulate" a Phy device (aka. RF device)
    │       ├── CMakeLists.txt
    │       ├── include
    │       └── src
    ├── FreeRTOSConfig.cmake : FreeRTOS configuration file for this application
    ├── STM32HALConfig.cmake : STM32 HAL configuration file for this board/bsp
    └── Nucleo-L476.cmake    : main cmake file to build and link everthing together


.. 
   Build system
   ============
   
   The build system is based on CMake tool which generate makefiles. Each directory to build shall include a *CMakeList.txt* file that gives rules "how to" and "what to " build.
   Furthermore, some other *cmake* files provide helping support for common or generic rules. These last files are located in the *tools/build_suuport* directory.     
   
   
   - toolchain-config.cmake
      This file gives rules to select the toolchain to use. It rely on *native_toolchain-config.cmake* and *cross_toolchain-config.cmake*. 
   
   - config_support-config.cmake and install_support-config.cmake files provides 
   
   
   - project_support-config.cmake
   
   
   
   Build Options
   -------------
   
   - DOC_COMPILE : Enable the Documentation compilation. Default is OFF.
   - BUILD_OPENWIZE : . Default is ON.
   - USE_FREERTOS : . Default is OFF.
   - BUILD_DEMO : . Default is OFF.
   - BUILD_TEST : . Default is OFF.
   - BUILD_UNITTEST : . Default is OFF.
   - BUILD_INTEGRATION_TEST : . Default is OFF.
   - BUILD_SYSTEM_TEST : . Default is OFF.
   - ENABLE_NATIVE_UNITTEST : Enable unit-test native execution. Default is OFF.
   - ENABLE_EMEBED_UNITTEST  : . Default is OFF.
   
   
   
   - USE_FREERTOS_SAMPLE : Enable the use of FreeRTOS sample provided by OpenWize. Default is ON)
   - USE_CRYPTO_SAMPLE : Enable the use of Crypto sample provided by OpenWize. Default is ON)
   - USE_CRC_SAMPLE : Enable the use of CRC_sw sample provided by OpenWize. Default is ON)
   - USE_REEDSOLOMON_SAMPLE : Enable the use of ReedSolomon sample provided by OpenWize. Default is ON)
   - USE_PARAMETERS_SAMPLE : Enable the use of Parameters sample provided by OpenWize. Default is ON)
   - USE_IMGSTORAGE_SAMPLE : Enable the use of ImgStorage sample provided by OpenWize. Default is ON)
   - USE_TIMEEVT_SAMPLE : Enable the use of TimeEvt sample provided by OpenWize. Default is ON)
   
   - IS_LOGGER_ENABLE : Enable the Logger in OpenWize. Default is ON)
   - USE_LOGGER_SAMPLE : Enable the use of Logger sample provided by OpenWize. Default is ON)
   
   
   
   Compilation options
   -------------------
   
   
   
   
   
   
   Porting guide
   =============
   
   PHY device
   ----------
   
   
   
   
   Board
   -----
   
   
   Compiler
   --------
    
   
   
   Extra
   =====


Modifing the default parameters
-------------------------------

(See "HowTo.md" in "tools/scripts/gen_param directory").

In "demo/Nucleo-L476/app/cfg, open and modify as you need the "DefaultParams.xml" file. Then, regenerate the default tables as explained in the following.


Setup the "gen_table.sh" script path in environment PATH variable:

.. code-block:: bash

   cd OpenWize
   export PATH=$PATH:$(pwd)/tools/scripts/gen_param
   cd demo/Nucleo-L476/app


Merge the two xml files :

.. code-block:: bash

   xmlmerge ./cfg/DefaultParams.xml ./cfg/DefaultRestr.xml -o ./cfg/MergedParam.xml

Then, call "gen_table.sh" :

.. code-block:: bash

   gen_table.sh --in ./cfg/MergedParam.xml --out .


It will generate 3 files in "gen" directory:
::::::::::::::::::::::::::::::::::::::::::::

   gen
   ├──parameters_cfg.h     : access and restriction table declaration
   ├──parameters_cfg.c     : access and restriction table definition
   └──parameters_default.c : define the tables default content

In these files : 

- *a_ParamAccess[]* : gives the parameters access right. 

This table describe for each parameter, its id, local access rights, remote access rights, update time, referenced, size, offset and restriction id (see the software documention for details).

- *a_ParamRestr[]* : gives the restrictions on parameters values (if any) 

This table describe for each restriction, its id, type (modulo, range, enum), the element size (8, 16, 32, 64 bits), the number of restriction element (modulo: 1; range: 2; enum: n éléments), adress of the element table (see the software documention for details).

- *a_ParamDefault[]* : gives the parameters defaults values

