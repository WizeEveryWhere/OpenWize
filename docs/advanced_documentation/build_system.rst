
Build system
============

Introduction
------------

The build system is based on CMake tool which generate makefiles. Each directory
to build shall include a *CMakeList.txt* file that gives rules "how to" and 
"what to " build. Furthermore, some other *cmake* files provide helping support 
for common or generic rules. These last files are located in the *tools/build_suport* 
directory.     

Build Support
-------------

toolchain-config.cmake
^^^^^^^^^^^^^^^^^^^^^^

This file gives rules to select the toolchain to use. It rely on *native_toolchain-config.cmake* and *cross_toolchain-config.cmake*. 

native_toolchain-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Define the minimum to be able to compile for native x86 target. 

cross_toolchain-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Define the arm-none-eabi-xx as cross-compiler to be used. 

config_support-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^
 
This file provide helper macros or functions to :

- *get_cfg* : this macro will include your own platform specific cmake. Based on command line argument *BUILD_CFG* and *BUILD_CFG_DIR*.
   
   - BUILD_CFG : 
      The file name of your specific cmake (e.g. -DBUILD_CFG=Nucleo-L476.cmake).
   
   - BUILD_CFG_DIR : 
      The path where is located your specific cmake (e.g. -DBUILD_CFG_DIR=demo/Nucleo-L476).
      The default path is "demo".

   Furthermore, this macro will extract the git information and produce the "version.h" file (see `git_info_config.cmake`_).
   
- *setup_config* : this function generate a *.h* file from *.in* file.  

   .. code-block:: cmake
      
      setup_config(
         NAME <file> 
         SOURCE <src/path> 
         DESTINATION <dest/path>
         )

   - <file>      : the file name (without extension) given for input and output.
   - <src/path>  : the source path of the "file".in 
   - <dest/path> : the destination where to write the "file".h

install_support-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 
- *setup_install* : helper function to create an install target
 
   .. code-block:: cmake
      
      setup_install(
         TARGET    <target>
         NAMESPACE <namesapce>
         DEPENDS   <depends>
         )

   - <target>    : the install target name to create
   - <namesapce> : optional namespace
   - <depends>   : a list of other target on which our target depend
         
git_info_config.cmake
^^^^^^^^^^^^^^^^^^^^^

This file provide functions to extract informations from git and generate a "version.h" file.

- *gitinfo* : extract the git information

   .. code-block:: cmake
   
      gitinfo( <working directory> )

   - <working directory> : base directory where to find git information. 


- *version_from_git* : generate the "version.h" file

   .. code-block:: cmake
      
      version_from_git( <config_file_path> <version_file> )

   Example of version.h content : 
   
   .. code-block:: c
            
         #ifndef _VERSION_H_
         #define _VERSION_H_
         
         #define GIT_RETRIEVED_STATE false 
         #define GIT_HEAD_SHA1 7bddf62f904a664a8c56be7904c13a8b57543222
         #define GIT_DESCRIBE 7bddf62
         #define GIT_IS_DIRTY 1
         
         #define GIT_TAG "UNK.99.99.99" // undefined
         #define GIT_FW_VER_TYPE "UNK"
         #define GIT_FW_VER_MAJ 99
         #define GIT_FW_VER_MIN 99
         #define GIT_FW_VER_REV 99
         
         #define GIT_AUTHOR_NAME ""
         #define GIT_AUTHOR_EMAIL ""
         
         #define GIT_COMMIT_DATE_ISO8601 "2022-02-22 07:45:43 +0100"
         #define GIT_COMMIT_SUBJECT "[FEATURE] Add the possibility to auto generate default parameters"
         #define GIT_COMMIT_BODY ""
         
         #endif /* _VERSION_H_ */


gen_param-config.cmake
^^^^^^^^^^^^^^^^^^^^^^

- *gen_param* : generate ".c" and ".h" parameter files from xml

   .. code-block:: cmake
      
      gen_param( 
         SOURCE <src/path> 
         DESTINATION <dest/path>
         )
   
   - <src/path>  : path where to find xml files. Both DefaultParams.xml and DefaultRestr.xml must be defined.
   - <dest/path> : path where the parameters files will be generated
   
   
  Command line option :
  
  - **GENERATE_PARAM** : boolean, default is OFF
      

.. *****************************************************************************
.. references

.. _`Doxygen`: https://www.doxygen.nl/manual/index.html




.. only:: comment

   Build Options
   -------------
   
   "project_support-config.cmake" is the "main" cmake file that give the rules to build the project. 
   
   Available options :
   
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
   
   
   
   
   
   

