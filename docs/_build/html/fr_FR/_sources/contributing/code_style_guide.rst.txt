
.. _coding style guide:

Coding Style Guide
==================

Conventions
-----------

**PascalCase** convention : a compound word that uses capital letters to delineate words. Includes the first letter.

- example : SomeName

**snake_case** convention : when all the words in a compound word are lower case but delimited by an underscore. 

- example : some_name


Directories Names
-----------------

- If the directory is a module directory, use 'pascal case' convention.
- If the directory is not a module directory, use 'snake case' convention.


Files Names
-----------

- Files name should use the *snake_case* convention. The first character of the name should be a letter and all others should be letters and/or numbers.
- C source file names must end in .c
- C header file names must end in .h
- Assembler source file names must end in .s


Include files
-------------

- Use the <name> construction for getting them from a standard place, and use "" for custom libraries or define them relative to the current directory. 

.. code-block:: c

   #include <stdio.h>
   #include "my_module_header.h"

Prefer the "include-path" option of the C compiler to handle extensive private libraries of header files. Never use absolute pathnames for header files.


Header file
-----------

- Header files should have preprocessor guards.

.. code-block:: c

      #ifndef _FILE_NAME_H_
      #define _FILE_NAME_H_
      ...
      #endif // _FILE_NAME_H_

- Always include check for C++ with **extern** keyword in header file. 

.. code-block:: c

      #ifdef __cplusplus
      extern "C" {
      #endif
      
      /* declarations go here */
      
      #ifdef __cplusplus
      }
      #endif


Note that the following order should be used : 
   - preprocessor guards
   - then any #include statements
   - then extern "C" guards:


Source file
-----------

- Always include check for C++ with **extern** keyword in header file. 

.. code-block:: c

      #ifdef __cplusplus
      extern "C" {
      #endif
      
      /* declarations go here */
      
      #ifdef __cplusplus
      }
      #endif


Module organization
-------------------

Softwares modules are organized with src/ and include/ directories, that 
respectively hold source and header files. In module root folder a "CMakeList.txt"
file defines one or more build target in order to be able to build that module. 
Furthermore, when is required and module is unitary "testable", a "unittest" 
directory containing unit test source code and its CMakeList.txt is present.
   
   .. rubric:: Typical module look like this :

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

Naming convention
-----------------

For simple type, we use the (u)intN_t convention from "stdint.h". That is to say, the basic type are (u)int8_t, (u)int16_t, (u)int32_t, (u)int64_t, float, double.

Variables Names
"""""""""""""""

The generic rule is that variables follow the PascalCase convention. Furthermore, variables are prefixed with their type.

For simple types, the variables are prefixed with their type in their "short" version :

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


The same rule is applied for structure, enum, pointer and array type. 

Example with structure and enum :

.. code-block:: c
   
   my_struct_t sMyVar; /* this a structure */
   my_enum_e   eMyVar; /* this an enum */


Example with pointer and array :

.. code-block:: c
   
   uint8_t *pMyVar;    /* this a pointer */
   uint8_t aMyVar[nb element]/* this an array */


.. note::
   Pointer is different from array or table in the sense that array is allocated, pointer is not.
    


Static variable follow the snake_case convention with *_* (underscore as prefix and suffix :

.. code-block:: c
   
   static uint8_t _u8_my_static_var_; /* this is my static variable */



Structures Names
""""""""""""""""

Structure declaration follows the snake_case convention suffixed with *_s* or *_t* if it is a type:

.. code-block:: c

   struct my_struct_s 
   {
      ...
   }
   
   typedef struct 
   {
      ...
   } my_struct_type_t


Structure instantiation follows the PascalCase convention prefixed with *s*:

.. code-block:: c

   struct my_struct_s sMyStruct; 
   my_struct_type_t sMyStructType;


Enumerate Names
"""""""""""""""

Enumerate declaration follows the snake_case convention suffixed with *_e* :

.. code-block:: c

   enum my_enum_e 
   {
      ...
   }
   
   typedef enum 
   {
      ...
   } my_enum_type_e



Enumerate instantiation follows the PascalCase convention prefixed with *e*:

.. code-block:: c

   my_enum_type_e eMyEnumType;


Functions Names
^^^^^^^^^^^^^^^

For public or API functions, its name is prefixed by the module name : 

.. code-block:: c
   
   void MyModuleName_MyFunctionName(...)

The module name prefix and function name use the PascalCase convention.

.. note::

   Sometimes, it is necessary or it could be better to be able to identify some structural functionality.  
   In these cases, use a structural name as identifier :

   For example, in case of a Board Support Package (BSP). The BSP is not really a 
   single module, but more a collection module. All BSP files are grouped under, 
   let say, "bsp" directory. Some of them will deal with flash memory while other 
   ones will deal with SPI.

   Example : 
  
   bsp_flash :
   
   .. code-block:: c
      
      BSP_Flash_Erase(...)

   bsp_spi :
   
   .. code-block:: c
      
      BSP_Spi_Read(...)

For private or static function, '_' (underscore) is added as prefix and suffix.

.. code-block:: c
   
   static void _my_function_name_(...)


Passed variable in function declaration/implementation must use the PascalCase convention.

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

Other General rules
-------------------

- Use *C99*
- Do not use tabs, use 4 spaces instead
- Use 1 space between keywords like *if*, *do*, *while*, *switch* and opening bracket
   
   .. code-block:: c
   
      if (condition)
      {
         ...
      }

- Use single space before and after comparison and assignment operators
   
   .. code-block:: c
   
      uint8_t i = 1;
      if (i == 3)
      {
         ...
      }

- Do not use space between function name and opening bracket
- Use single space after every comma
- Do not initialize **static** and **global** variables to 0 (or *NULL*), let compiler do it for you
- Avoid variable assignment with function call in declaration
- Always use brackets with **sizeof** operator
- Use **const** for *pointer* if function should not modify memory pointed to by *pointer*
- Use **const** for function parameter or variable, if it should not be modified

.. code-block:: c
    
      /* When pData could be modified, data pointed to by pData could not be modified */
      void my_func(const void* pData) {
      ...
      }
      
      /* When pData should not be modified inside function, only data pointed to by pData
       * could be modified 
       */
      void my_func(void* const pData) {
      ...
      }
      
      /* When pData and data pointed to by pData both could not be modified */
      void my_func(const void* const pData) {
      ...
      }

- Preprocessor macro use always upper case and, if required, underscore.

   .. code-block:: c
   
         #define MY_DEFINE

- Use ``//`` or ``/* * /`` block for single line comments. 
   
   .. code-block:: c
       
         // My one line comment
         /* My other one line comment */


- Use ``/* */`` block for multi-line comments.
   
   .. code-block:: c
       
         /* 
          * My multi line comment
          */


- Try to avoid global variable.  
- Always respect code style already used in project or library (even if style is not clear or not obvious)
- Every function must include doxygen-enabled comment, even if function is **static**
- Use English names/text for functions, variables, comments
- Commits should only contain files with LF (Unix style) endings.

