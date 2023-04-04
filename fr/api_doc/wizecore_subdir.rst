.. *****************************************************************************

Sous-répertoire *WizeCore*
----------------------------

``proto``
   *proto* implémente tout ce qui est nécessaire pour traiter les couches lien et présentation du protocole Wize, c'est à dire :

   - construire/extraire les informations des couches lien et présentation
   - chiffrer/déchiffrer le message de l'application
   - calculer et vérifier le hachage d'authentification 
   - calculer et vérifier le hachage de confidentialité
   - calculer et vérifier le CRC
   - calculer et vérifier le code Reed-Solomon et corriger la trame si possible (couche téléchargement uniquement).

   *proto* dépend des bibliothèques "CRC_sw", "Crypto" et "ReedSolomon".


``net``
   *net* implémente une sorte de pilote de bas niveau pour : 

   - piloter le protocole dans *proto*
   - fournir une couche abstraite au niveau supérieur, avec des fonctions comme "send(...)" ou "recev(...)".
   - fournir une interface à l'implémentation du pilote phy

   *net* dépend de *proto* 

``mgr``
   *mgr* implémente quatre gestionnaires :

   - *adm_mgr*, *inst_mgr* et *dwn_mgr* gèrent respectivement les sessions d'administration, d'installation et de téléchargement. Leur rôle est principalement d'assurer la synchronisation des sessions.
   
   - *net_mgr* est une sorte de pilote de haut niveau pour le module *net*. En particulier, il gère les ressources de net+phy de manière sûre et garantit la synchronisation des fenêtres d'envoi et d'écoute.

   *mgr* dépend de *net*, *TimeEvt*, *Logger* et *FreeRTOS*.

``app``
   *app* implémente la couche application du protocole Wize et fournit une API au reste du firmware de l'application.

   - fournit une API pour envoyer un message DATA et exécuter une session d'installation (PING/PONG)
   - ouvrir/fermer des sessions d'administration, d'installation et de téléchargement.
   - construire/extraire la couche d'administration générique (aka DATA, COMMAND, RESPONSE)
   - construire/extraire la couche d'installation (aka PING/PONG)
   - traiter l'image téléchargée du firmware (stocker, valider)

   *app* dépend de *mgr*, *TimeEvt*, *ImgStorage*, *Parameters*, *Logger* et *FreeRTOS*.

.. *****************************************************************************
