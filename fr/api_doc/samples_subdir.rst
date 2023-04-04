.. *****************************************************************************

Sous-répertoire *Samples*
--------------------------

``CRC_sw``
   La bibliothèque *CRC_sw* fournit des fonctions pour calculer et vérifier le CRC16 (*Cyclic Redundancy Check*) comme spécifié par `Wize Lan Protocol Specifications`_ v1.2.

``Crypto``
   La bibliothèque *Crypto* fournit des fonctions d'interface pour chiffrer, déchiffrer, calculer, etc. 
   AES-CMAC et SHA256. Ce module s'appuie sur la bibliothèque *tinycrypt* située dans le répertoire "third-party/libraries/Tinycrypt" (https://github.com/intel/tinycrypt.git).

``ReedSolomon``
   La bibliothèque *ReedSolomon* implémente l'algorithme de correction du code Reed-Solomon 
   RS(255,223). Ce module est un portage de la bibliothèque http://www.eccpage.com/rs.c

``ImgStorage``
   La bibliothèque *ImgStorage* fournit une interface pour stocker les blocs de microprogrammes téléchargés sur la mémoire physique. 

``Parameters``
   La bibliothèque *Parameters* fournit des interfaces pour définir et obtenir des paramètres. 

``TimeEvt``
   *Le module TimeEvt* fournit un événement à la tâche au moment requis.  

``Logger``
   Le module *Logger* permet d'enregistrer les messages (info, warning, error, debug).
   

.. *****************************************************************************
.. references
.. _`Wize Lan Protocol Specifications`: https://www.wize-alliance.com/Downloads/Technical
