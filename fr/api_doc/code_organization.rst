.. *****************************************************************************

Organisation du code
=====================

Structure du référentiel
-------------------------

La structure du dépôt est la suivante :

::

   OpenWize
   ├── docs : répertoire de documentation
   ├── demo 
   │   ├── Nucleo-L476 : contient le code source spécifique à la démo et à la carte Nucleo-L476.
   │   │   ├── app    : Une application de démonstration simple.
   │   │   ├── board  : Spécifique à la carte Nucleo L476
   │   │   ├── bsp    : Demo specific board support package for Nucleo L476
   │   │   └── device : Pilote de périphérique spécifique à la démonstration.
   │   └── project : Fichiers de projet IDE
   ├── sources : contient le code source spécifique à l'OpenWize  
   │   ├── Samples  : code source des bibliothèques, fournies comme "samples", requises par le WizeCore.
   │   └── WizeCore : code source de la stack Wize
   ├── third-party :
   │   ├── firmware
   │   │   └── STM32     : STM32 HAL comme un sous-module
   │   ├── libraries
   │   │   └── Tinycrypt : Bibliothèque Tinycrypt comme sous-module
   │   ├── rtos
   │   │   └── FreeRTOS  : FreeRTOS comme un sous-module
   │   └── testing
   │       └── Unity     : CMock comme un sous-module
   ├── tools
   │   ├── build_support : Fichiers cmake pour aider à la construction d'OpenWize
   │   └── scripts       : divers scripts bash
   │
   └── CMakeLists.txt : Le fichier principal CMakeList.txt


Répertoire des sources
-----------------------

.. rubric::  Le répertoire *sources* est organisé comme suit :

Il y a principalement deux sous-répertoires "Samples" et "WizeCore". "WizeCore" est l'implémentation de la l'implémentation de la stack, tandis que "Samples" contient quelques bibliothèques nécessaires à la stack Wize. Le WizeCore est divisé en quatre dossiers, chacun correspondant plus ou moins à une couche d'abstraction.

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

Annuaire de démonstrations
---------------------------

::

    Nucleo-L476
    ├── app : application code
    │   ├── CMakeLists.txt
    │   ├── cfg     : contient les fichiers xml de configuration des paramètres par défaut 
    │   ├── gen     : contient les tables de paramètres par défaut sous forme de fichiers .c et .h
    │   ├── include : répertoire des bibliothèques de l'application
    │   ├── src     : répertoire des sources de l'application
    │   └── sys     : initialiser les modules système (RTOS, Logger, Stack...)
    ├── board : Contient le minimum pour initialiser la carte à bas niveau (périphériques, horloges)
    │   ├── CMakeLists.txt
    │   ├── include : 
    │   ├── ld      :  
    │   ├── src     : 
    │   └── startup : 
    ├── bsp : Paquet de soutien restreint et simple pour le conseil d'administration
    │   ├── CMakeLists.txt
    │   ├── include
    │   └── src
    ├── device 
    │   └── PhyFake : Enveloppe autour de l'UART pour "simuler" un périphérique Phy (aka. RF device)
    │       ├── CMakeLists.txt
    │       ├── include
    │       └── src
    ├── FreeRTOSConfig.cmake : Fichier de configuration FreeRTOS pour cette application.
    ├── STM32HALConfig.cmake : Fichier de configuration STM32 HAL pour cette carte/bsp.
    └── Nucleo-L476.cmake    : Fichier cmake principal pour construire et lier le tout.

.. include:: demo_subdir.rst

.. *****************************************************************************
