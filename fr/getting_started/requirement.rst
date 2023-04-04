
Exigences
===========

La stack OpenWize est essentiellement développée pour cibler les MCU de STMicroelectronic, basées sur le noyau ARM-M. La stack OpenWize a été développée pour cibler les MCU de STMicroelectronic. sur un cœur ARM-M. Pour des raisons de commodité, nous utilisons le STM32CubeIDE comme IDE. 
Il est livré avec tout ce qui est nécessaire pour compiler, charger et déboguer le firmware sur ce MCU ST. 
Cet IDE peut être téléchargé depuis le site web de STMicroelectronics (`STM32CubeIDE`_).

Le système de compilation est basé sur les outils *cmake* et *make* qui s'appuient sur la chaîne d'outils croisée fournie par STMicroelectronics. 
la chaîne d'outils croisée fournie par le STM32CubeIDE. Nous fournissons un petit script bash "set_env.sh" dans "tools/script" pour vous aider à configurer certaines variables d'environnement.

Restriction
------------

L'ensemble du développement et des tests a été réalisé sous le système d'exploitation Linux, Windows n'est pas supporté.

Version minimale
-----------------

Le tableau suivant donne les versions minimales recommandées pour pouvoir construire la stack OpenWize.  

.. list-table:: 
   :widths: 20 30 50

   * - Host
     - Linux Ubuntu 18.04
     - There are no special requirement. It should works on any Linux distribution.
   * - Git
     - 2.24
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

Notez que `Git`_, `Cmake`_ et `Make`_ sont généralement déjà installés sur Linux de base, il n'y a donc rien de plus à faire.

.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Cmake`: https://cmake.org
.. _`Git`: https://git-scm.com
.. _`Make`: https://www.gnu.org/software/make
.. _`Alciom` : https://www.alciom.com/en/home
.. _`Wize’Up`: https://www.alciom.com/en/our-trades/products/wizeup
.. _`Wize'Up AT Specification`: https://www.alciom.com/wp-content/uploads/2021/05/RL2031-008-wizeup-AT-command-specification-1E.pdf 

