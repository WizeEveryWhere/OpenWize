
Requirement
===========

The OpenWize stack is basically developed to target STMicroelectronic MCU, based
on ARM-M cpu core. For convenience reasons, we use the STM32CubeIDE as an IDE. 
It come with all necessary to compile, load and debug firmware on that ST MCU. 
The used version is "1.1.0" but latest one should be compatible. IDE can be 
download from STMicroelectronics web-site (https://www.st.com/en/development-tools/stm32cubeide.html#get-software).

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

Note that *Git*, *Cmake* and *Make* are usually already installed on basic Linux 
distribution, so there is nothing more to do.

