
First steps
===========

Cloning this repository
-----------------------

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


Install the STM32Cube IDE
-------------------------

.. |STMCubeIdeVersion| replace:: 1.8.0
.. |STMCubeIdeExecutable| replace:: st-stm32cubeide_1.8.0_11526_20211125_0815_amd64.sh
.. |PreferdInstPath| replace:: /opt/Application/st/stm32cubeide_1.8.0

.. only:: comment
   .. |STMCubeIdeVersion| replace:: 1.1.0
   .. |STMCubeIdeExecutable| replace:: st-stm32cubeide_1.1.0_4551_20191014_1140_amd64.sh
   .. |PreferdInstPath| replace:: /opt/Application/st/stm32cubeide_1.1.0

Go to the STMicroelectronic web page (`STM32CubeIDE`_), on *STM32CubeIDE Generic Linux Installer* line, select the version |STMCubeIdeVersion| and download it. 
Unzip the zip file somewhere in temporary directory, then execute the resulting file ("|STMCubeIdeExecutable|") and follow the instruction. 

Note that preferred installation path as "|PreferdInstPath|". 

After few minutes, the STM32CubeIDE installation is completed. 

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


It's also possible to "un-install" :

.. code-block:: bash

   cd OpenWize/tools/scripts
   ./set_env.sh -u 

   
.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software

