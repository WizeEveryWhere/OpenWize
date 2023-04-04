
Application de démonstration
=============================

- Cette application est une simple démonstration de la stack OpenWize sur une carte ST Nucleo-L476RG. 

- La carte Nucleo-L476RG n'intègre pas de dispositif RF, donc pour pallier à cela, l'application de démonstration utilise le dispositif ``PhyFake``, qui entre/sort des trames via un périphérique UART.

- Cette démo est très simple, il n'y a donc pas de console ou d'interface locale pour communiquer avec elle. Au lieu de cela, l'application envoie périodiquement des messages DATA et INSTALL (aka. PING) et utilise le module *Logger* fourni pour imprimer des messages (info, warning, debug, error). 

- Le *Logger* est connecté au périphérique USART2 du MCU.

   - Configuration : 115200 bps, 8 bits de données, pas de parité, 1 bit d'arrêt.
   - Ce USART est mappé sur CN1 (USB-mini).

- Le *PhyFake* est connecté au périphérique LPUART1 du MCU :

   - Configuration : 115200 bps, 8 bits de données, pas de parité, 1 bit de stop.
   
   - Ce USART est mappé sur : 
   
      - RX : PC0 (CN7-38)
      - TX : PC1 (CN7-36).

- Certains paramètres par défaut ont été réglés sur :

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


Créer l'application
---------------------

.. code-block:: bash

   cd Openwize
   mkdir -p _build
   cd _build

.. code-block:: bash

   cmake -DBUILD_CFG=Nucleo-L476/Nucleo-L476 -DCMAKE_BUILD_TYPE=Debug ../. 
   make DemoApp -j
   make install
   
Les fichiers du firmware sont installés dans le répertoire OpenWize/*_install*.

:: 

   _install/
      └── bin
          ├── DemoApp.elf  : The "elf" file (i.e. with debug symbols)
          ├── DemoApp.bin  : Pure binary file
          ├── DemoApp.lst  : Disassembly listing
          └── DemoApp.map  : Symbols and files mapping


Charger and exécuter the firmware
----------------------------------

Pour pouvoir charger et exécuter l'application de démonstration, vous aurez besoin d'une carte ST Nucleo-L476
et d'un convertisseur USB vers UART.

#. Connectez votre carte Nucleo-L476RG à votre ordinateur.
#. Connectez le convertisseur USB vers UART à votre ordinateur
#. Vérifier l'identification de la sonde ST-Link

   .. code-block:: bash

      cd OpenWize
      STM32_Programmer_CLI -l

   Vous devriez obtenir quelque chose comme ça :

   .. code-block::
   
            -------------------------------------------------------------------
                              STM32CubeProgrammer v2.9.0-RC01                  
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

   Dans cet exemple, la sonde id id 0 :
   
   :: 

      ST-Link Probe 0 :
      ST-LINK SN  : 066CFF383333554157243011
      ...


   Ensuite, téléchargez le firmware sur la carte
   
   .. code-block:: bash

      STM32_Programmer_CLI -c port=SWD mode=UR reset=HWrst index=0 -d _install/bin//DemoApp.bin 0x08000000


   Vous devriez pouvoir voir les messages *Logger* sur *ttyACM0* et les images Wize sur *ttyUSB0*.


Déboguer l'application de démonstration
----------------------------------------

Dans le répertoire "OpenWize/demo/project", nous trouvons les fichiers du projet Eclipse. 
Ouvrez l'IDE STMCube32 et *Importer* comme *Projets existants dans l'espace de travail*.



.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
