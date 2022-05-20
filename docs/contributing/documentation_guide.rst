
.. _documentation guide:

Documentation Guide
===================

Files
-----

Each source and header file must begin with the following :  

.. code-block:: 
   :linenos:

   /*!
     * @file filename.[c/h]
     * @brief A brief file description
     * 
     * @details Optionaly, describe details about this file
     *
     * @copyright 2022, Company Name, Inc.  All rights reserved.
     *
     * Redistribution and use in source and binary forms, with or without 
     * modification, are permitted (subject to the limitations in the disclaimer
     * below) provided that the following conditions are met:
     *    - Redistributions of source code must retain the above copyright notice,
     *      this list of conditions and the following disclaimer.
     *    - Redistributions in binary form must reproduce the above copyright 
     *      notice, this list of conditions and the following disclaimer in the 
     *      documentation and/or other materials provided with the distribution.
     *    - Neither the name of GRDF, Inc. nor the names of its contributors
     *      may be used to endorse or promote products derived from this software
     *      without specific prior written permission.
     *
     *
     * @par Revision history
     *
     * @par 1.0.0 : 2021/03/01 [ABC]
     * Initial version
     * 
     * @par 1.0.1 : 2022/03/01 [CBA]
     * Changes in function X to take into account the feature Z
     *
     */

Revision history is formatted as follow :

.. code-block:: 

   * @par MAJ.MIN.REV : DATE [AUTHOR]
   * Describe the revisions changes 

Where, "MAJ.MIN.REV" is respectively major, nimor and revision version number. 
This must be followed by the date and author. Then on next line, describe the revision changes.

Functions
---------

All function must begin with its documentation block :

.. code-block:: 

   /*!
     * @brief Brief documentation of this function
     *
     * @details Optionaly, give some details about this function.
     *
     * @param [in]      in_param    Input Parameter description.
     * @param [out]     out_param   Output Parameter description.
     * @param [in, out] inout_param Input/Output Parameter description.
     *
     * @retval return value x if everything is fine
     *         return value y if something goes wrong
     */

If the function is private add *@static* at the begining, just before *@brief*.

.. code-block:: 

   /*!
     * @static
     * @brief ...
     *

If required, hie some documentation part under *@cond* block. 

.. code-block:: 

   /*!
    * @cond INTERNAL
    * @{
    */
   
   Here the content that should appear in documention only if INTERNAL is defined
   
   /*!
    * @}
    * @endcond
    */

In order to organize the documentation, add a group to your module.

.. code-block:: 

   /*!
    * @addtogroup groupe_name
    * @{
    *
    */
      
      ... body of this module ...
      
   /*! @} */

Structures and enumeration
--------------------------

.. code-block:: 

   /*!
    * @brief Describe your enum 
    */
   typedef enum{
      ENUM_0,        /*!< Enum 0 decritption */
      ENUM_1,        /*!< Enum 0 decritption */
      
      /* Optionaly add a "marker" */
      ENUM_NB 
   } my_enum_e;


.. code-block:: 

   /*!
    * @brief Describe your structure
    */
   typedef struct  {
      uint8_t u8Field1;  /*! Field 1 description */
      int16_t i16Field2; /*! Field 2 description */
   } my_struct_t


.. code-block:: 

   /*!
    * @brief 
    */
   #define MY_DEFINE 1

.. code-block:: 

   /*!
    * @brief 
    */
   extern my_struct_t sMyStruct;


.. code-block:: 

   /*!
    * @static
    * @brief 
    */
   static my_struct_t _my_struct_;


.. *****************************************************************************
.. references

.. _`Doxygen`: https://www.doxygen.nl/manual/index.html
.. _`Doxygen Style`: https://www.doxygen.nl/manual/docblocks.html#specialblock
