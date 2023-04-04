
Guide de portage
=================

..
   Ajouter une application à la démo Nucleo L476
   ------------------------------------------
   TBD 

Portage de votre dispositif PHY
--------------------------------

Jetez un coup d'œil au dispositif de démonstration *PhyFake* qui donne un exemple simple.

Préparer votre module
^^^^^^^^^^^^^^^^^^^^^

- Créez un répertoire pour votre dispositif PHY.
- Créez le fichier *include/phy_layer_private.h*.
- Créez les fichiers *src/phy_layer.c* et *src/phy_layer_private.c*.
- Créer le fichier *CMakeLists.txt*

Cela devrait donner quelque chose comme :

:: 

   MyPhy
   ├── CMakeLists.txt
   ├── include
   │   └── phy_layer_private.h
   └── src
       ├── phy_layer_private.c
       └── phy_layer.c

Implémenter le pilote de bas niveau
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Dans *phy_layer_private.h* et *phy_layer_private.c* intégrez toutes les exigences (fonction, defines, typedef, ...) pour piloter à bas niveau votre périphérique. Son contenu dépend fortement du comportement de votre périphérique. 

Implémenter le pilote phy
^^^^^^^^^^^^^^^^^^^^^^^^^

Le *phy_layer.c* est destiné à implémenter l'interface phy d'OpenWize. 

Interface phy
"""""""""""""

Inclure la définition de l'interface phy dans *phy_itf.h*.
Définir l'implémentation de l'interface :

.. code-block:: c

   static const phy_if_t _phy_if = {
      .pfInit          = _my_init_,
      .pfUnInit        = _my_uninit_,
      .pfTx            = _my_TX_,
      .pfRx            = _my_RX_,
      .pfNoise         = _my_noise_,
      .pfSetSend       = _my_set_send_,
      .pfGetRecv       = _my_get_recv_,
      .pfIoctl         = _my_ioctl_
   };

Implémenter les fonctions de l'interface :

- **_my_init_** : Fonction d'initialisation du pilote.

   .. code-block:: c
      
      static int32_t _my_init_(phydev_t *pPhydev);
 
   Cette fonction doit initialiser : 
   
   - La structure interne *pPhydev*.
   - La communication sur le bus bsp (s'il y en a une).
   - La structure interne de votre périphérique *my_phy_device_t*.

- **_my_uninit_**   : Fonction de désinitialisation du pilote.

   .. code-block:: c
      
      static int32_t _my_uninit_(phydev_t *pPhydev)
   
   Cette fonction doit désinitialiser : 
   
   - la communication sur le bus bsp (s'il y en a une).

- **_my_TX_**       : Réalise l'action de transmission

   .. code-block:: c
         
         static int32_t _my_TX_(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);
   
   Cette fonction doit :
   
   - Prendre en compte les changements de canal et de modulation.
   - Appeler la fonction d'envoi de bas niveau.  

- **_my_RX_**       : Réalise l'action d'écoute

   .. code-block:: c

      static int32_t _my_RX_(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);

   Cette fonction doit :
   
   - Prendre en compte les changements de canal et de modulation.
   - Appeler la fonction d'écoute à bas niveau.

- **_my_noise_**    : Mesure le bruit

   .. code-block:: c

      static int32_t _my_noise_(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation);

   Cette fonction est optionnelle 

- **_my_set_send_** : Transfère la trame dans le tampon d'émission interne de l'appareil

   .. code-block:: c

      static int32_t _my_set_send_(phydev_t *pPhydev, uint8_t *pBuf, uint8_t u8Len);
   
   Cette fonction doit : 
   
   - Transférer le contenu de pBuf dans le tampon interne de l'appareil.

- **_my_get_recv_** : Récupère la trame dans le tampon de réception interne de l'appareil.

   .. code-block:: c

      static int32_t _my_get_recv_(phydev_t *pPhydev, uint8_t *pBuf, uint8_t *u8Len);

   Cette fonction doit : 
   
   - Obtenir le contenu du tampon interne de l'appareil et le copier dans pBuf.

- **_my_ioctl_**    : Obtenir/Régler la configuration de l'appareil  

   .. code-block:: c

      static int32_t _my_ioctl_(phydev_t *pPhydev, uint32_t eCtl, uint32_t args);
   
   Le contrôle des E/S phy est donné par *eCtl* tandis que *args* est la valeur à obtenir/paramétrer. Voir phy_ctl_e pour plus d'informations. 
   

Fonction de configuration
""""""""""""""""""""""""""

Cette fonction est destinée à préparer l'appareil Phy à une configuration constante.

.. code-block:: c
   
   int32_t Phy_MyPhy_setup(phydev_t *pPhydev, my_phy_device_t *pCtx)


L'implémentation minimale est la suivante :

.. code-block:: c

   int32_t Phy_MyPhy_setup(phydev_t *pPhydev, my_phy_device_t *pCtx)
   {
      int32_t i32Ret = PHY_STATUS_ERROR;
      if (pPhydev && pCtx)
      {
         pPhydev->pIf = &_phy_if;
         pPhydev->pCxt = pCtx;
         i32Ret = PHY_STATUS_OK;
      }
       return i32Ret;
   }

Gestionnaire d'interruptions
"""""""""""""""""""""""""""""

Ce gestionnaire doit être appelé lorsqu'une interruption se produit à partir de votre appareil et doit ressembler à ce qui suit :

.. code-block:: c

   static void _my_frame_it_(void *p_CbParam, void *p_Arg)
   {
      uint32_t eEvt = PHYDEV_EVT_NONE;
      phydev_t *pPhydev = (phydev_t *) p_CbParam;
      my_phy_device_t *pDevice;
      uint32_t u32IrqStatus = MY_PHY_EVT_NONE;
      
      if (pPhydev)
      {
         pDevice = pPhydev->pCxt;
         // ...faire quelque chose avec la structure privée, si nécessaire
         // Obtenir l'état de l'irq 
         // ...à partir de l'argument de passage 
         // u32IrqStatus = ((uint32_t)p_Arg) ;
         // ...ou en appelant une fonction de bas niveau
         // u32IrqStatus = MyPhy_GetIrq(...) ;
      }
      
      if (u32IrqStatus == MY_PHY_EVT_RX_STARTED)
      {
         eEvt = PHYDEV_EVT_RX_STARTED ;
         // ...mettre en œuvre une action sur cet événement, le cas échéant
      }
      if (u32IrqStatus == MY_PHY_EVT_TX_CPLT)
      {
         eEvt = PHYDEV_EVT_TX_COMPLETE ;
         // ...mettre en œuvre une action sur cet événement, le cas échéant
      }
      if (u32IrqStatus == MY_PHY_EVT_RX_CPLT)
      {
         eEvt = PHYDEV_EVT_RX_COMPLETE ;
         // ...mettre en œuvre une action sur cet événement, le cas échéant
      }
      
      // notification de l'événement
      if( (eEvt != PHYDEV_EVT_NONE) && pPhydev->pfEvtCb ) 
      {
         // Notifier le niveau supérieur
         pPhydev->pfEvtCb(pPhydev->pCbParam, eEvt) ;
      }
   }

Port sur un nouveau tableau
----------------------------

TBD

..
   Requirement : 
   
   - board : TBD
   - bsp : TBD
   - device : TBD
   - application : TBD

Compilateur
------------
TBD
