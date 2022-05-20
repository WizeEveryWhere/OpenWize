.. *****************************************************************************

Code Organization
=================

Repository structure
--------------------

The repository structure is the following

::

   OpenWize
   ├── docs : documentation directory
   ├── demo 
   │   ├── Nucleo-L476 : contains source code specific to the Demo and Nucleo-L476 board
   │   │   ├── app    : A simple demo application
   │   │   ├── board  : Nucleo L476 board specific
   │   │   ├── bsp    : demo specific board support package for Nucleo L476
   │   │   └── device : demo specific device driver
   │   └── project : IDE project files
   ├── sources : contains source code specific to the OpenWize 
   │   ├── Samples  : source code of libraries, provided as "samples", required by the WizeCore
   │   └── WizeCore : Wize Stack source code
   ├── third-party :
   │   ├── firmware
   │   │   └── STM32     : STM32 HAL as a submodule
   │   ├── libraries
   │   │   └── Tinycrypt : Tinycrypt library as a submodule
   │   ├── rtos
   │   │   └── FreeRTOS  : FreeRTOS as a submodule
   │   └── testing
   │       └── Unity     : CMock as a submodule
   ├── tools
   │   ├── build_support : cmake files to help building OpenWize
   │   └── scripts       : various bash script
   │
   └── CMakeLists.txt : the main CMakeList.txt file


Sources directory
-----------------

.. rubric::  The *sources* directory is organized as follow :

There are mainly two sub-directories "Samples" and "WizeCore". "WizeCore" is the 
stack implementation, while "Samples" contains some required libraries by the 
Wize stack. The WizeCore is split in four folders, each one corresponding more 
or less to an abstraction layer. 

::

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
   ├── WizeCore
   │   ├── app
   │   ├── mgr
   │   ├── net
   │   ├── proto
   │   └── CMakeLists.txt
   ├── CMakeLists.txt
   └── OpenWize_Options.cmake

.. include:: samples_subdir.rst
.. include:: wizecore_subdir.rst

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
    │   └── sys     : initialize the system modules (RTOS, Logger, Stack...)
    ├── board : Contains the minimum to initialize the board low level (peripherals, clocks) 
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
    └── Nucleo-L476.cmake    : main cmake file to build and link everything together

.. include:: demo_subdir.rst

.. *****************************************************************************
