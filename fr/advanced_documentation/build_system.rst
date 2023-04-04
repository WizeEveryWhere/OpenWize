
Système de construction
========================

Introduction
-------------

Le système de construction est basé sur l'outil CMake qui génère des fichiers makefiles. Chaque répertoire à construire doit inclure un fichier *CMakeList.txt* qui donne les règles "comment" et "quoi construire". En outre, d'autres fichiers *cmake* fournissent un support d'aide pour les règles communes ou génériques. Ces derniers fichiers sont situés dans le répertoire *tools/build_suport* dans le répertoire *tools/build_suport*.     

Aide à la construction
-----------------------

toolchain-config.cmake
^^^^^^^^^^^^^^^^^^^^^^

Ce fichier donne des règles pour sélectionner la chaîne d'outils à utiliser. Il s'appuie sur *native_toolchain-config.cmake* et *cross_toolchain-config.cmake*. 

native_toolchain-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Définit le minimum pour pouvoir compiler pour une cible x86 native. 

cross_toolchain-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Définit le compilateur croisé arm-none-eabi-xx à utiliser. 

config_support-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^
 
Ce fichier fournit des macros ou des fonctions d'aide pour :

   - *get_cfg* : cette macro inclura votre propre cmake spécifique à votre plateforme. Basée sur les arguments de la ligne de commande *BUILD_CFG* et *BUILD_CFG_DIR*.
   
   - BUILD_CFG : 
      Le nom du fichier de votre cmake spécifique (par exemple -DBUILD_CFG=Nucleo-L476.cmake).
   
   - BUILD_CFG_DIR : 
      Le chemin où se trouve votre cmake spécifique (par exemple -DBUILD_CFG_DIR=demo/Nucleo-L476).
      Le chemin par défaut est "demo".

   De plus, cette macro va extraire les informations git et produire le fichier "version.h" (voir `git_info_config.cmake`_).
   
- *setup_config* : cette fonction génère un fichier *.h* à partir d'un fichier *.in*.  

   .. code-block:: cmake
      
      setup_config(
         NAME <file> 
         SOURCE <src/path> 
         DESTINATION <dest/path>
         )

   - <file> : le nom du fichier (sans extension) donné pour l'entrée et la sortie.
   - <src/path> : le chemin source du "fichier".in 
   - <dest/path> : la destination où écrire le "fichier".h

install_support-config.cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 
- *setup_install* : fonction d'aide pour créer une cible d'installation
 
   .. code-block:: cmake
      
      setup_install(
         TARGET    <target>
         NAMESPACE <namesapce>
         DEPENDS   <depends>
         )

   - <target> : nom de la cible d'installation à créer
   - <namesapce> : espace de noms optionnel
   - <depends> : une liste d'autres cibles dont notre cible dépend
         
git_info_config.cmake
^^^^^^^^^^^^^^^^^^^^^

Ce fichier fournit des fonctions pour extraire des informations de git et génère un fichier "version.h".

- *gitinfo* : extrait les informations de git

   .. code-block:: cmake
   
      gitinfo( <working directory> )

   - <working directory> : répertoire de base où se trouvent les informations git.  


- *version_from_git* : génère le fichier "version.h".

   .. code-block:: cmake
      
      version_from_git( <config_file_path> <version_file> )

   Exemple de contenu du fichier version.h :
   
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

- *gen_param* : générer des fichiers de paramètres ".c" et ".h" à partir de xml

   .. code-block:: cmake
      
      gen_param( 
         SOURCE <src/path> 
         DESTINATION <dest/path>
         )
   
   - <src/path> : chemin où se trouvent les fichiers xml. Les fichiers DefaultParams.xml et DefaultRestr.xml doivent tous deux être définis.
   - <dest/path> : chemin où les fichiers de paramètres seront générés.
   
   
  Option de ligne de commande :
  
  - **GENERATE_PARAM** : booléen, la valeur par défaut est OFF
      

.. *****************************************************************************
.. references

.. _`Doxygen`: https://www.doxygen.nl/manual/index.html




.. only:: commentaire

   Options de construction
   ------------------------
   
   "project_support-config.cmake" est le fichier cmake "principal" qui donne les règles de construction du projet. 
   
   Options disponibles :
   
   - DOC_COMPILE : Active la compilation de la documentation. La valeur par défaut est OFF.
   - BUILD_OPENWIZE : . La valeur par défaut est ON.
   - USE_FREERTOS : . La valeur par défaut est OFF.
   - BUILD_DEMO : . La valeur par défaut est OFF.
   - BUILD_TEST : . La valeur par défaut est OFF.
   - BUILD_UNITTEST : . La valeur par défaut est OFF.
   - BUILD_INTEGRATION_TEST : . La valeur par défaut est OFF.
   - BUILD_SYSTEM_TEST : . La valeur par défaut est OFF.
   - ENABLE_NATIVE_UNITTEST : Active l'exécution native des tests unitaires. La valeur par défaut est OFF.
   - ENABLE_EMEBED_UNITTEST : . La valeur par défaut est OFF.
   
    
   - USE_FREERTOS_SAMPLE : Active l'utilisation de l'échantillon FreeRTOS fourni par OpenWize. La valeur par défaut est ON)
   - USE_CRYPTO_SAMPLE : Active l'utilisation de l'échantillon Crypto fourni par OpenWize. La valeur par défaut est ON)
   - USE_CRC_SAMPLE : Active l'utilisation de l'échantillon CRC_sw fourni par OpenWize. La valeur par défaut est ON)
   - USE_REEDSOLOMON_SAMPLE : Active l'utilisation de l'échantillon ReedSolomon fourni par OpenWize. La valeur par défaut est ON)
   - USE_PARAMETERS_SAMPLE : Active l'utilisation de l'échantillon Parameters fourni par OpenWize. La valeur par défaut est ON)
   - USE_IMGSTORAGE_SAMPLE : Active l'utilisation de l'échantillon ImgStorage fourni par OpenWize. La valeur par défaut est ON)
   - USE_TIMEEVT_SAMPLE : Active l'utilisation de l'échantillon TimeEvt fourni par OpenWize. La valeur par défaut est ON)
   
   - IS_LOGGER_ENABLE : Active le Logger dans OpenWize. La valeur par défaut est ON)
   - USE_LOGGER_SAMPLE : Active l'utilisation de l'échantillon Logger fourni par OpenWize. La valeur par défaut est ON)
      
      
      
   Options de compilation
   -----------------------
   
   
   
   
   
   

