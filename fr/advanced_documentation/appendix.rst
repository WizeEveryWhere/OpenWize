

Annexe
========

Modification des paramètres par défaut
---------------------------------------

(Voir "HowTo.md" dans le répertoire "tools/scripts/gen_param").

Dans "demo/Nucleo-L476/app/cfg", ouvrez et modifiez comme vous le souhaitez le fichier "DefaultParams.xml".
et modifiez-le selon vos besoins. Ensuite, régénérez les tables par défaut comme expliqué dans la suite.

Prérequis : 
L'outil "xmlstarlet" doit être installé. (sudo apt-get install xmlstarlet)
L'outil "xmlmerge" doit être installé. (sudo apt-get install gwenhywfar-tools)

Configurer le chemin du script "gen_table.sh" dans la variable PATH de l'environnement :

.. code-block:: bash

   cd OpenWize
   export PATH=$PATH:$(pwd)/tools/scripts/gen_param
   cd demo/Nucleo-L476/app


Fusionner les deux fichiers xml :

.. code-block:: bash

   xmlmerge ./cfg/DefaultParams.xml ./cfg/DefaultRestr.xml -o ./cfg/MergedParam.xml

Ensuite, appelez "gen_table.sh" :

.. code-block:: bash

   gen_table.sh --in ./cfg/MergedParam.xml --out .


.. rubric::  Il génère 3 fichiers dans le répertoire "gen" :

::

   gen
   ├──parameters_cfg.h     : déclaration du tableau d'accès et de restriction
   ├──parameters_cfg.c     : définition du tableau d'accès et de restriction
   └──parameters_default.c : défini le contenu par défaut des tableaux

Dans ces fichiers : 

- *a_ParamAccess[]* : donne les droits d'accès aux paramètres. 

Ce tableau décrit pour chaque paramètre, son identifiant, ses droits d'accès locaux, ses droits d'accès droits d'accès à distance, l'heure de mise à jour, la référence, la taille, l'offset et l'identifiant de restriction (voir la documentation du logiciel pour plus de détails). (voir la documentation du logiciel pour plus de détails).

- *a_ParamRestr[]* : donne les restrictions sur les valeurs des paramètres (s'il y en a). 

Ce tableau décrit pour chaque restriction, son identifiant, son type (modulo, range, enum), la taille de l'élément (8, 16, 32, 64 bits), le nombre d'éléments de restriction (modulo : 1 ; range : 2 ; enum : n éléments), l'adresse du tableau des éléments (pour plus de détails, voir la (voir la documentation du logiciel pour plus de détails).

- a_ParamDefault[]* : donne les valeurs par défaut des paramètres.




