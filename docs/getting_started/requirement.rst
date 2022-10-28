
Requirement
===========

The OpenWize stack is basically developed to target STMicroelectronic MCU, based
on ARM-M cpu core. For convenience reasons, we use the STM32CubeIDE as an IDE. 
It comes with all necessary to compile, load and debug firmware on that ST MCU. 
This IDE can be downloaded from STMicroelectronics website (`STM32CubeIDE`_).

The build system is based on *cmake* and *make* tools which rely on the 
cross-toolchain provided by the STM32CubeIDE. We provide a little bash script 
"set_env.sh" in "tools/script" to help you to setup some environment variables.

Restriction
-----------

The whole development and tests have been done under Linux operating system, 
Windows is not supported.

Minimum version
---------------

The following table gives the minimum recommended versions to be able to build 
the OpenWize stack.

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

Note that `Git`_, `Cmake`_ and `Make`_ are usually already installed on basic Linux 
Distribution, so there is nothing more to do.

.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Cmake`: https://cmake.org
.. _`Git`: https://git-scm.com
.. _`Make`: https://www.gnu.org/software/make
.. _`Alciom` : https://www.alciom.com/en/home
.. _`Wize’Up`: https://www.alciom.com/en/our-trades/products/wizeup
.. _`Wize'Up AT Specification`: https://www.alciom.com/wp-content/uploads/2021/05/RL2031-008-wizeup-AT-command-specification-1E.pdf 

