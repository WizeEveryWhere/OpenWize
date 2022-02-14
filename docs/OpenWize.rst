
.. sectnum::

########
OpenWize
########


***********
Requirement
***********

The OpenWize stack is basically developed to target STMicroelectronic MCU, based on ARM-M cpu core. For convenience reasons, we use the STM32CubeIDE as an IDE. It come with all necessary to compile, load and debug firmware on that ST MCU. The used version is "1.1.0" but latest one should be compatible. IDE can be download from STMicroelectronics web-site (https://www.st.com/en/development-tools/stm32cubeide.html#get-software).

The build system is based on *cmake* and *make* tools which rely on the cross-toolchain provided by the STM32CubeIDE. We provide a little bash script "set_env.sh" in "tools/script" to help you to setup some environment variables.

Restriction
===========

The whole development and tests have been done under Linux operating system, Windows is not supported.

Minimum version
===============

The following table gives the minimum recommanded versions to be able to build the OpenWize stack.

.. list-table:: 
   :widths: 20 30 50

   * - Host
     - Linux Ubuntu 18.04
     - There are no special requirement. It should works on any Linux distribution.
   * - Git
     - 2.34
     - There are no special requirement.
   * - Cmake
     - 3.13.2
     - The CMakeLists.txt and *xxx*.cmake files are based on cmake version greater equal than 3.12
   * - Make
     - 4.1
     - There are no special requirement.
   * - Cross-toolchain
     - 7.3.1 (arm-non-eabi-gcc) and 2.30.0 (binutils)
     - There are no special requirement.
   * - Newlib
     - 3.0.0
     - This is the versions packaged with the cross-toolchain. 

Note that *Git*, *Cmake* and *Make* are usally already installed on basic Linux distribution, so there is nothing more to do.


***********
First steps
***********

Cloning this repository
=======================
.. code-block:: bash

   git clone --recurse-submodules https://github.com/GRDF/OpenWize.git

.. with selecting "develop" branch: git clone -b develop --recurse-submodules https://github.com/GRDF/OpenWize.git

or alternatively 

.. code-block:: bash

   git clone https://github.com/GRDF/OpenWize.git
   cd OpenWize
   git submodule init --recursive
   git submodule update --recursive
   
.. with selecting "develop" branch: git clone -b develop https://github.com/GRDF/OpenWize.git


This will also clone the following required submodules :

.. list-table:: 
   :widths: 20 30 50 30
   :header-rows: 1
   
   * - Submodule
     - Version
     - Destination (from third-party/)
     - sha
   * - FreeRTOS
     - V10.4.4
     - rtos/FreeRTOS/FreeRTOS-Kernel
     - 8de8a9da1aa9b036812a72fdcd7cbdefc2789365
   * - cmsis core
     - v5.4.0_cm4
     - firmware/STM32/cmsis_core
     - cb6d9400754e6c9050487dfa573949b61152ac99
   * - cmsis device L4
     - v1.5.1
     - firmware/STM32/cmsis_device_l4
     - f42a6c319cc887c8a13f171d347294f2eabfab3b
   * - stm32l4xx_hal_driver
     - v1.10.0
     - firmware/STM32/stm32l4xx_hal_driver
     - 2737a6e3fdefa41570a29321afb6cd9c1de69b1c
   * - CMock
     - v2.5.2
     - testing/Unity/CMock
     - 150573c742ce15061a0b675aa0f8e29c85008062
   * - Tinycrypt
     - master branch
     - libraries/Tinycrypt/tinycrypt
     - 5969b0e0f572a15ed95dc272e57104faeb5eb6b0


Repository structure
====================
::

   OpenWize
   ├── docs : documentation directory
   ├── demo 
   │   ├── Nucleo-L476 : contains source code specific to the Demo application and Nucleo-L476 board
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


Prepare to build
================

Installation
------------

Go to the STMicroelectronic web page (`STM32CubeIDE`_), on "STM32CubeIDE Generic Linux Installer" line, select the version 1.1.0 and download it. Unzip the zip file somewhere in temporary directory, then execute the resulting file ("st-stm32cubeide_1.1.0_4551_20191014_1140_amd64.sh") and follow the instruction. Note that prefered installation path as ""/opt/Application/st". After few minutes, the STM32CubeIDE installation is completed. 

Setup environment variables
---------------------------

Two environment are required :

- CROSS_TOOL_PATH : give the main path of the cross-toolchain, which is used and required by the cmake build system.
- CUBE_PROG_PATH : give the main path of the STM32Cube programmer tool. This tool is use to upload the binary firmware on the target board, so not absolutely required during debugging stage.

The easy way to set these variable is to run the provided script in OpenWize/tools/scripts. This script will create all necessary to set these environment variable on new console or terminal.

In a console run the script : 

.. code-block:: bash

   cd OpenWize/tools/scripts
   ./set_env.sh -i

and follow the instructions.


It's also possible to "uninstall" :

.. code-block:: bash

   cd OpenWize/tools/scripts
   ./set_env.sh -u 


Demo application
================

`OpenWize Demo Application`_


.. Simulation x86
.. ==============
.. `OpenWize x86 Simulation`_

Advanced documentation
======================

`OpenWize Advanced Documentation`_


--------------------------------------------

- '#' with overline, for parts
- '*' with overline, for chapters
- '=' for sections
- '-' for subsections
- '^' for subsubsections
- '"' for paragraphs


.. references

.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Installation`: INSTALLATION.rst
.. _`Wize Lan Protocol Specifications`: https://www.wize-alliance.com/Downloads/Technical

.. _`OpenWize Demo Application`: https://github.com/GRDF/OpenWize/blob/main/docs/OpenWize_NucleoL476.rst
.. _`OpenWize x86 Simulation`: https://github.com/GRDF/OpenWize/blob/main/docs/OpenWize_Simu-x86.rst
.. _`OpenWize Advanced Documentation`: https://github.com/GRDF/OpenWize/blob/main/docs/OpenWize_Advanced.rst

