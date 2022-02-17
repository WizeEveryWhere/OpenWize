.. OpenWize documentation master file, created by
   sphinx-quickstart on Tue Nov 23 12:28:41 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Bienvenue sur OpenWize !
========================

Wize c’est quoi ?
-----------------

Lancé en mars 2017 par GRDF, Suez et Sagemcom, Wize est un protocole ouvert dont le principe repose sur une communication à distance basse consommation, bas débit (entre 1200 et 6400 bits/s) et longue portée.

Les spécifications du protocole Wize sont maintenues au sein de l’alliance Wize qui est une association à but non lucratif visant à promouvoir l’adoption de ce protocole au sein des réseaux IOT dans le monde de l’industrie et des services publics urbains. L’alliance Wize réunit en 2021 une quarantaine de membres. 

Wize utilise des ondes radio sur une fréquence 169 MHz ouverte et libre de droits en Europe afin de garantir la meilleure des pénétrations pour récolter des données de télémesure dans le domaine de l’Internet des objets.

Toute la documentation du protocole est accessible gratuitement sur le site de `l’Alliance Wize <https:\\www.wize-alliance.com>`_, ainsi que toutes les informations nécessaires pour devenir membre en quelques clics.

Pourquoi un stack 169 MHz en open source ?
------------------------------------------

GRDF a commencé ses travaux autour de la fréquence 169 MHZ en 2010 dans le cadre de son projet de télérelève de compteurs communicants Gaz. Le challenge que devait relever GRDF dans le cadre de ce projet était de mettre en place une infrastructure radio sur une bande de fréquence ouverte et libre de droits en Europe, afin de permettre la télérelève des compteurs de gaz de ses 11 millions de clients en s’appuyant sur :
 * Une technologie fiable lui permettant de garantir un taux de relève journalier d’au moins 95%,
 * Une durée de vie des équipements autonome sur pile de 20 ans,
 * Une communication bidirectionnelle permettant l’envoi d’ordre et le téléchargement à distance de firmware à l’intérieur de ses capteurs,
 * Une communication radio sûre et sécurisée de bout en bout.

Aucun LPWAN à l’époque (et c’est encore le cas en 2021) ne permettant de répondre à la totalité de ces contraintes, GRDF a pris la décision de développer son propre protocole de communication radio, en s’appuyant sur le standard W-MBUS mode N2 du comité technique européen 294, en y intégrant les fonctionnalités qui manquaient à l’époque à savoir la bidirectionnalité et les mécanismes de sécurisation de la communication.

Avec plus de 8 millions de compteurs déployés à mi-2021 et devant les excellentes performances obtenues à l’aide de la technologie mise en place, GRDF a décidé de participer à la promotion l’alliance wize en concevant et en développant une pile de protocole Wize 100% compatible avec les spécifications 1.2 de l’alliance et de publier ce développement en mode open source sous licence MIT. 

Par cette publication, GRDF a pour objectif d’accélérer l’adoption de cette technologie par le plus grand nombre et de faciliter la conception et la mise sur le marché d’un écosystème de capteurs IOT compatibles Wize le plus riche et le plus large possible pour répondre à ses propres besoins, mais également à ceux éventuels d’autres opérateurs industriels et de gestionnaires de services urbains publics.

OpenWize, le nom donné à cette couche protocolaire, a été développé pour etre le plus agnostique possible par rapport au matériel.

Son architecture modulaire sous FreeRTOS permet cependant d’envisager un portage rapide vrs une cible matériel de son choix. En adoptant une démarche open source, vise à enrichir l’écosystème de membres de l’alliance Wize et apportera son soutien à tous les contributeurs qui souhaiteraient s’investir dans les améliorations fonctionnelles ou le portage d’OpenWize.

OpenWize est disponible en téléchargement, mais peut-être également être intégré dans un module hardware open source Wiz’Up développé par la société Alciom (www.alciom.com). Dans cette dernière configuration, une société qui découvre la technologie Wize, possède tous les éléments pour accélérer son projet IOT. 

OpenWize est constitué d'un écosystème qui s'appuie sur deux repositories :
 * le repository OpenWize XXXX qui contient le stack en lui meme qui s'accompagne d'un example rapide de mise en oeuvre.
 * le repository OpenWize'Up YYYY qui contient l'instanciation du stack OpenWize sur la carte Wize'Up développée par un membre de l'Alliance Wize, la société ALCIOM.
  
Getting started
---------------

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


.. references

.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Installation`: INSTALLATION.rst
.. _`Wize Lan Protocol Specifications`: https://www.wize-alliance.com/Downloads/Technical

.. _`OpenWize Demo Application`: https://github.com/GRDF/OpenWize/blob/main/docs/OpenWize_NucleoL476.rst
.. _`OpenWize x86 Simulation`: https://github.com/GRDF/OpenWize/blob/main/docs/OpenWize_Simu-x86.rst
.. _`OpenWize Advanced Documentation`: https://github.com/GRDF/OpenWize/blob/main/docs/OpenWize_Advanced.rst


Les cartes de développement disponibles pour utiliser OpenWize
--------------------------------------------------------------

Wiz’up : un module Wize haute performances et ouvert, la solution idéale pour développer simplement un produit compatible avec les réseaux LPWAN très longue distance Wize.

Wiz’Up est le fruit d’un appel à projet de l’alliance wize, dans lequel les sociétés Alciom et GRDF ont réunies leurs savoirs-faires et expertises pour offrir une solution clés en main d’accélération de projets IOT. Le concept est de fournir un module de communication équipé d’OpenWize, facilement intégrable dans un nouveau design hardware. Ce module s’interface facilement via une liaison UART par l’intermédiaire d’un ensemble de commandes AT. Pour plus d’information sur `Wiz’Up <https://www.alciom.com/nos-métiers/produits/wizeup/>`_

Pour plus d'information sur la mise en oeuvre du stack OpenWize sur la carte Wize'Up : (rajouter lien car l'index du readTheDocs de OpenWize'Up)

Vous trouverez également ci-desous un example simple d'utilisation d'OpenWize sur une carte de démonstration STM Nucléo L476RG qui ne possède pas de transceiver RF (la trame sort sur une UART de la carte).

Quels réseaux 169 MHz utiliser avec la technologie Wize
-------------------------------------------------------
Wize est une technologie utilisable en mode « dédié » et en mode « opéré ».

En mode « dédié », un client décide d’installer sa propre infrastructure de concentrateurs Wize et s’enregistre en tant qu’opérateur auprès de l’alliance wize. Si vous développez un produit pour un client de ce type, votre client votre communiquera toutes les informations nécessaires pour l’enregistrement de votre produit sur son réseau.

En mode « opéré », un client décide d’utiliser le réseau d’un opérateur existant afin de faire remonter les données de ses capteurs vers son système d’information. Si vous développez un produit pour un client de ce type, votre client doit vous mettre en relation avec l’opérateur wize qu’il a sélectionné afin que vous puissiez enregistrer votre produit sur son réseau. La liste des opérateurs wize ainsi que les points de contacts associés sont disponibles sur le site de l’alliance.

A noter qu’à partir de la version 1.2, pour faciliter le développement de produit wize, l’alliance wize a introduit le concept Wiz’One qui permet de faire transiter ponctuellement des données sur tous les réseaux wize compatibles 1.2 à des fins de tests unitaires ou d’intégration. Le support de Wiz’One est dépendant de l’opérateur et peut-être associé à certaines restrictions en terme de fonctionnalités.

Pour aller plus loin dans votre développement
---------------------------------------------

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
   TBD
   
   Porting guide
   =============
   
   PHY device
   ----------
   TBD
   
   Board
   -----
   TBD

   Compiler
   --------
   TBD

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

Un exemple simple de mise en oeuvre sur une carte STM Nucléo L476RG
-------------------------------------------------------------------

- This application is provide as a simple demonstrator of OpenWize stack targeting an ST Nucleo-L476RG board. 
- The Nucleo-L476RG board doesn't integrate any RF device, so to overcome this, the demo application use the ``PhyFake`` device, which input/output frames over an UART peripheral.
- This demo is very simple so, there is no console or local interface to communicate with it. Instead, application periodically send DATA and INSTALL (aka. PING) message and use the provided *Logger* module to print out messages (info, warning, debug, error). 
- The *Logger* is connected to the MCU USART2 peripheral :
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
   mkdir _build

.. code-block:: bash

   cmake -DBUILD_CFG=Nucleo-L476/Nucleo-L476 -DCMAKE_BUILD_TYPE=Debug ../. 
   make DemoApp -j
   make install
   
The firmware files are installed in OpenWize/*_install* directory

:: 

   _install/
      └── bin
          ├── DemoApp      : The "elf" file (i.e. with debug symbols)
          ├── DemoApp.bin  : Pure binary file
          ├── DemoApp.lst  : Disassembly listing
          └── DemoApp.map  : Symbols and files mapping


Load and run the firmware
-------------------------
To be able to load and run the demo application, you will need an ST Nucleo-L476 board and and USB-to-UART converter.

#. Connect your Nucleo-L476RG board to your computer
#. Connect the USB-to-UART to your computer
#. Check ST-Link probe id

   .. code-block:: bash

      cd OpenWize
      export PATH=$PATH:$(pwd)/tools/scripts/test_support
      twk_load_stlink.sh -h

   You should get something like that :

   .. code-block::

      Error! Give a probe index to be able to continu.
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
      
.. .........

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

In the directory "OpenWize/demo/project", we provides Eclipse project files. Open the STMCube32 IDE and *Import* as *Existing Projects into Workspace*.


--------------------------------------------

.. references

.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Installation`: INSTALLATION.rst


Guide pour contribuer à OpenWize.
---------------------------------
Les contributions à OpenWize – Bugs fix, ajout de nouvel fonctionnalités, ajout de documentation, ajouts d’exemples – sont les bienvenus. Ces contributions doivent s’effectuer sous la forme de Github Pull Request.

Prés-requis pour contribuer :
 * Contribution sous la même licence que celle d’OpenWize.
 * Votre code doit être conforme au style de développement d’OpenWize (à décrire)
 * Votre code doit être conforme au style de documentation d’OpenWize (à décrire)
 * Votre code doit être suffisamment documenter pour que sa structure soit facilement compréhensible.
 * Votre code est-il accompagné d’une documentation ou d’exemples qui permettent d’en comprendre facilement l’utilisation.
 * Les commentaires de votre code et votre documentation sont-ils écrits en anglais.

User Guide de l'écriture du code
--------------------------------
TBD

Si vous n’êtes pas sûr de l’ensemble de ces points, n’hésitez pas à ouvrir un issue sur le repository d'OpenWize pour nous poser vos questions.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
