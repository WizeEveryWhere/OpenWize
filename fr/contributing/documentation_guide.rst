
.. _documentation guide:

Guide de documentation
=======================

Fichiers
---------

Chaque fichier source et fichier d'en-tête doit commencer par la mention suivante :  

.. code-block:: 
   :linenos:

   /*!
     * @file Nom du fichier.[c/h]
     * @brief Brève description du fichier
     * 
     * @details Optionnel, décrire les détails de ce fichier
     *
     * @copyright 2022, Company Name, Inc.  Tous droits réservés.
     *
     * La redistribution et l'utilisation sous forme source et binaire, avec ou sans modification, sont autorisées (sous réserve des limitations énoncées dans la clause de non-responsabilité). 
     * modification, sont autorisées (sous réserve des limitations énoncées dans l'avis de non-responsabilité
     * ci-dessous) à condition que les conditions suivantes soient remplies :
     * - Les redistributions du code source doivent conserver l'avis de droit d'auteur ci-dessus,
     * la présente liste de conditions et la clause de non-responsabilité suivante.
     Les redistributions sous forme binaire doivent reproduire l'avis de copyright ci-dessus, * cette liste de conditions et la clause de non-responsabilité suivante. 
     * ci-dessus, la présente liste de conditions et l'avis de non-responsabilité suivant dans la 
     * dans la documentation et/ou les autres éléments fournis avec la distribution.
     * - Ni le nom de GRDF, Inc. ni les noms de ses contributeurs ne peuvent être utilisés pour approuver ou soutenir un produit ou un service.
     * ne peuvent être utilisés pour approuver ou promouvoir des produits dérivés de ce logiciel
     * sans autorisation écrite préalable.
     *
     *
     * @par Historique de la révision
     *
     * @par 1.0.0 : 2021/03/01 [ABC]
     * Version initiale
     * 
     * @par 1.0.1 : 2022/03/01 [CBA]
     * Changements dans la fonction X pour prendre en compte la fonctionnalité Z
     *
     */

L'historique des révisions est présenté comme suit :

.. code-block:: 

   * @par MAJ.MIN.REV : DATE [AUTHOR]
   * Décrire les modifications apportées aux révisions  

Où "MAJ.MIN.REV" est respectivement le numéro de version majeure, mineure et de révision. 
Ce numéro doit être suivi de la date et de l'auteur. Ensuite, sur la ligne suivante, décrivez les changements de révision.

Fonctions
----------

Toutes les fonctions doivent commencer par leur bloc de documentation :

.. code-block:: 

   /*!
     * @brief Brève documentation de cette fonction
     *
     * @details Optionnel, donnez quelques détails sur cette fonction.
     *
     * @param [in] in_param Description du paramètre d'entrée.
     * @param [out] out_param Description du paramètre de sortie.
     * @param [in, out] inout_param Description des paramètres d'entrée/sortie.
     *
     * @retval valeur de retour x si tout va bien
     * valeur de retour y si quelque chose ne va pas
     */

Si la fonction est privée, ajoutez *@static* au début, juste avant *@brief*.

.. code-block:: 

   /*!
     * @static
     * @brief ...
     *

Si nécessaire, placez une partie de la documentation sous le bloc *@cond*.

.. code-block:: 

   /*!
    * @cond INTERNAL
    * @{
    */
   
   Voici le contenu qui doit apparaître dans la documentation uniquement si l'option INTERNEL est définie.
   
   /*!
    * @}
    * @endcond
    */

Afin d'organiser la documentation, ajoutez un groupe à votre module.

.. code-block:: 

   /*!
    * @addtogroup groupe_name
    * @{
    *
    */
      
      ... corps du module ...
      
   /*! @} */

Structures et énumération
---------------------------

.. code-block:: 

   /*!
    * @brief Décrivez votre enum 
    */
   typedef enum{
      ENUM_0,        /*!< Enum 0 decritption */
      ENUM_1,        /*!< Enum 0 decritption */
      
      /* Possibilité d'ajouter un "marqueur" */
      ENUM_NB 
   } my_enum_e;


.. code-block:: 

   /*!
    * @brief Décrivez votre structure
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
