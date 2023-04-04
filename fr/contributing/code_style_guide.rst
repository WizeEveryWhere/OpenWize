
.. _coding style guide:

Guide de style de codage
=========================

Conventions
------------

Convention **PascalCase** : mot composé qui utilise des lettres majuscules pour délimiter les mots. Inclut la première lettre.

- exemple : SomeName

Convention **snake_case** : lorsque tous les mots d'un mot composé sont en minuscules mais délimités par un trait de soulignement. 

- exemple : quelque_nom


Noms des répertoires
---------------------

- Si le répertoire est un répertoire de modules, utilisez la convention 'pascal case'.
- Si le répertoire n'est pas un répertoire de modules, utilisez la convention 'snake case'.


Fichiers Noms
--------------

- Le nom des fichiers doit utiliser la convention *snake_case*. Le premier caractère du nom doit être une lettre et tous les autres doivent être des lettres et/ou des chiffres.
- Les noms des fichiers source C doivent se terminer par .c
- Les noms des fichiers d'en-tête C doivent se terminer par .h
- Les noms des fichiers source de l'assembleur doivent se terminer par .s


Fichiers inclus
----------------

- Utilisez la construction <nom> pour les obtenir à partir d'un emplacement standard, et utilisez "" pour les bibliothèques personnalisées ou définissez-les par rapport au répertoire actuel.

.. code-block:: c

   #include <stdio.h>
   #include "my_module_header.h"

Préférez l'option "include-path" du compilateur C pour gérer les bibliothèques privées étendues de fichiers d'en-tête. N'utilisez jamais de noms de chemin absolus pour les fichiers d'en-tête.


Fichier d'en-tête
------------------

- Les fichiers d'en-tête doivent avoir des gardes de préprocesseur.

.. code-block:: c

      #ifndef _FILE_NAME_H_
      #define _FILE_NAME_H_
      ...
      #endif // _FILE_NAME_H_

- Toujours inclure la vérification pour C++ avec le mot-clé **extern** dans le fichier d'en-tête.

.. code-block:: c

      #ifdef __cplusplus
      extern "C" {
      #endif
      
      /* les déclarations vont ici */
      
      #ifdef __cplusplus
      }
      #endif


Notez que l'ordre suivant doit être utilisé : 
   - les gardes du préprocesseur
   - puis toutes les déclarations #include
   - puis les protections extern "C" :


Fichier source
---------------

- Toujours inclure la vérification pour C++ avec le mot-clé **extern** dans le fichier d'en-tête.

.. code-block:: c

      #ifdef __cplusplus
      extern "C" {
      #endif
      
      /* les déclarations vont ici */
      
      #ifdef __cplusplus
      }
      #endif


Organisation du module
-----------------------

Les modules de logiciels sont organisés avec les répertoires src/ et include/, qui contiennent respectivement les fichiers sources et les fichiers d'en-tête. Dans le dossier racine du module, un fichier "CMakeList.txt" définit une ou plusieurs cibles de construction afin de pouvoir construire ce module. 
En outre, lorsque cela est nécessaire et que le module est unitairement "testable", un répertoire "unittest" contenant le code source du test unitaire et son fichier CMakeList.txt est présent.
   
   .. rubric:: Un module type ressemble à ceci :

:: 

   Module
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

Convention d'appellation
-------------------------

Pour les types simples, nous utilisons la convention (u)intN_t de "stdint.h". En d'autres termes, les types de base sont (u)int8_t, (u)int16_t, (u)int32_t, (u)int64_t, float, double.

Noms des variables
"""""""""""""""""""

La règle générique est que les variables suivent la convention PascalCase. En outre, les variables sont préfixées par leur type.

Pour les types simples, les variables sont préfixées par leur type dans leur version "courte" :

.. code-block:: c
   
   uint8_t u8MyVar; 
   int8_t i8MyVar;
   uint16_t u16MyVar; 
   int16_t i16MyVar;
   uint32_t u32MyVar; 
   int32_t i32MyVar;
   uint64_t u64MyVar; 
   int64_t i64MyVar;
   float fMyVar; 
   double dMyVar;


La même règle s'applique aux types structure, enum, pointeur et tableau. 

Exemple avec structure et enum :

.. code-block:: c
   
   my_struct_t sMyVar; /* Ceci est une structure */
   my_enum_e   eMyVar; /* Ceci est une énumération */


Exemple avec un pointeur et un tableau :

.. code-block:: c
   
   uint8_t *pMyVar;    /* Ceci est un pointeur */
   uint8_t aMyVar[nb element]/* Ceci est un tableau */


.. note::
   Le pointeur est différent du tableau ou de la table dans le sens où le tableau est alloué, ce qui n'est pas le cas du pointeur.
    


Les variables statiques suivent la convention snake_case avec *_* (underscore comme préfixe et suffixe :

.. code-block:: c
   
   static uint8_t _u8_my_static_var_; /* c'est ma variable statique */



Noms des structures
""""""""""""""""""""

La déclaration d'une structure suit la convention snake_case suffixée par *_s* ou *_t* s'il s'agit d'un type:

.. code-block:: c

   struct my_struct_s 
   {
      ...
   }
   
   typedef struct 
   {
      ...
   } my_struct_type_t


L'instanciation des structures suit la convention PascalCase préfixée par *s* :

.. code-block:: c

   struct my_struct_s sMyStruct; 
   my_struct_type_t sMyStructType;


Dénombrer les noms
"""""""""""""""""""

La déclaration d'énumération suit la convention snake_case suffixée par *_e* :

.. code-block:: c

   enum my_enum_e 
   {
      ...
   }
   
   typedef enum 
   {
      ...
   } my_enum_type_e



L'instanciation des énumérations suit la convention PascalCase préfixée par *e* :

.. code-block:: c

   my_enum_type_e eMyEnumType;


Noms des fonctions
^^^^^^^^^^^^^^^^^^^

Pour les fonctions publiques ou API, son nom est préfixé par le nom du module : 

.. code-block:: c
   
   void MyModuleName_MyFunctionName(...)

Le préfixe du nom du module et le nom de la fonction utilisent la convention PascalCase.

.. note::

   Il est parfois nécessaire ou préférable de pouvoir identifier une fonctionnalité structurelle.  
   Dans ce cas, il convient d'utiliser un nom de structure comme identifiant :

   Par exemple, dans le cas d'un Board Support Package (BSP). Le BSP n'est pas vraiment un un module unique, mais plutôt un module de collection. Tous les fichiers BSP sont regroupés dans le répertoire, disons, le répertoire "bsp". Certains d'entre eux traitent de la mémoire flash tandis que d'autres traitent du SPI. traiteront de la mémoire flash tandis que d'autres traiteront du SPI.

   Exemple : 
  
   bsp_flash :
   
   .. code-block:: c
      
      BSP_Flash_Erase(...)

   bsp_spi :
   
   .. code-block:: c
      
      BSP_Spi_Read(...)

Pour les fonctions privées ou statiques, "_" (trait de soulignement) est ajouté comme préfixe et suffixe.

.. code-block:: c
   
   static void _my_function_name_(...)


La variable passée dans la déclaration/implémentation de la fonction doit utiliser la convention PascalCase.

.. code-block:: c
   
   void MyModuleName_MyFunctionName(uint8_t u8MyVarName);
   
   void MyModuleName_MyFunctionName(uint8_t u8MyVarName)
   {
      ...
   }
   
   static void _my_function_name(uint8_t u8MyVarName);
   
   
   static void _my_function_name(uint8_t u8MyVarName)
   {
      ...
   }

Autres règles générales
------------------------

- Utiliser *C99*
- Ne pas utiliser de tabulations, utiliser 4 espaces à la place
- Utilisez 1 espace entre les mots-clés comme *if*, *do*, *while*, *switch* et le crochet d'ouverture.
   
   .. code-block:: c
   
      if (condition)
      {
         ...
      }

- Utiliser un seul espace avant et après les opérateurs de comparaison et d'affectation
   
   .. code-block:: c
   
      uint8_t i = 1;
      if (i == 3)
      {
         ...
      }

- Ne pas utiliser d'espace entre le nom de la fonction et la parenthèse ouvrante.
- Utiliser un espace après chaque virgule
- Ne pas initialiser les variables **statiques** et **globales** à 0 (ou *NULL*), laisser le compilateur le faire pour vous
- Éviter l'affectation de variables avec un appel de fonction dans la déclaration
- Utilisez toujours des parenthèses avec l'opérateur **sizeof**.
- Utilisez **const** pour *pointer* si la fonction ne doit pas modifier la mémoire pointée par *pointer*.
- Utilisez **const** pour le paramètre ou la variable de la fonction, s'ils ne doivent pas être modifiés

.. code-block:: c
    
      /* Lorsque pData a pu être modifié, les données pointées par pData n'ont pas pu être modifiées */
      void my_func(const void* pData) {
      ...
      }
      
      /* Lorsque pData ne doit pas être modifié à l'intérieur d'une fonction, seules les données pointées par pData
       * peuvent être modifiées 
       */
      void my_func(void* const pData) {
      ...
      }
      
      /* Lorsque pData et les données pointées par pData n'ont pas pu être modifiées */
      void my_func(const void* const pData) {
      ...
      }

- Les macros du préprocesseur sont toujours utilisées en majuscules et, si nécessaire, avec un trait de soulignement.

   .. code-block:: c
   
         #define MY_DEFINE

- Utilisez le bloc ``//`` ou ``/* * /`` pour les commentaires d'une seule ligne. 
   
   .. code-block:: c
       
         // Mon commentaire d'une ligne
         /* Mon autre commentaire d'une ligne */


- Utilisez le bloc ``/* */`` pour les commentaires sur plusieurs lignes.
   
   .. code-block:: c
       
         /* 
          * Mon commentaire en plusieurs lignes
          */


- Essayez d'éviter les variables globales.  
- Respectez toujours le style de code déjà utilisé dans le projet ou la bibliothèque (même si le style n'est pas clair ou évident).
- Chaque fonction doit inclure un commentaire compatible avec doxygen, même si la fonction est **statique**.
- Utiliser des noms/textes en anglais pour les fonctions, les variables et les commentaires.
- Les commits ne doivent contenir que des fichiers avec des terminaisons LF (style Unix).

