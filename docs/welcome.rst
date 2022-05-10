.. *****************************************************************************

************************
Bienvenue sur OpenWize !
************************

Wize c’est quoi ?
=================

Lancé en mars 2017 par GRDF, Suez et Sagemcom, Wize est un protocole ouvert dont le principe repose sur une communication à distance basse consommation, bas débit (entre 1200 et 6400 bits/s) et longue portée.

Les spécifications du protocole Wize sont maintenues au sein de l’alliance Wize qui est une association à but non lucratif visant à promouvoir l’adoption de ce protocole au sein des réseaux IOT dans le monde de l’industrie et des services publics urbains. L’alliance Wize réunit en 2021 une quarantaine de membres. 

Wize utilise des ondes radio sur une fréquence 169 MHz ouverte et libre de droits en Europe afin de garantir la meilleure des pénétrations pour récolter des données de télémesure dans le domaine de l’Internet des objets.

Toute la documentation du protocole est accessible gratuitement sur le site de `l’Alliance Wize <https:\\www.wize-alliance.com>`_, ainsi que toutes les informations nécessaires pour devenir membre en quelques clics.

Pourquoi un stack 169 MHz en open source ?
==========================================

GRDF a commencé ses travaux autour de la fréquence 169 MHZ en 2010 dans le cadre de son projet de télérelève de compteurs communicants Gaz. Le challenge que devait relever GRDF dans le cadre de ce projet était de mettre en place une infrastructure radio sur une bande de fréquence ouverte et libre de droits en Europe, afin de permettre la télérelève des compteurs de gaz de ses 11 millions de clients en s’appuyant sur :
 * Une technologie fiable lui permettant de garantir un taux de relève journalier d’au moins 95%,
 * Une durée de vie des équipements autonome sur pile de 20 ans,
 * Une communication bidirectionnelle permettant l’envoi d’ordre et le téléchargement à distance de firmware à l’intérieur de ses capteurs,
 * Une communication radio sûre et sécurisée de bout en bout.

Aucun LPWAN à l’époque (et c’est encore le cas en 2021) ne permettant de répondre à la totalité de ces contraintes, GRDF a pris la décision de développer son propre protocole de communication radio, en s’appuyant sur le standard W-MBUS mode N2 du comité technique européen 294, en y intégrant les fonctionnalités qui manquaient à l’époque à savoir la bidirectionnalité et les mécanismes de sécurisation de la communication.

Avec plus de 8 millions de compteurs déployés à mi-2021 et devant les excellentes performances obtenues à l’aide de la technologie mise en place, GRDF a décidé de participer à la promotion l’alliance wize en concevant et en développant une pile de protocole Wize 100% compatible avec les spécifications 1.2 de l’alliance et de publier ce développement en mode open source sous licence MIT. 

Par cette publication, GRDF a pour objectif d’accélérer l’adoption de cette technologie par le plus grand nombre et de faciliter la conception et la mise sur le marché d’un écosystème de capteurs IOT compatibles Wize le plus riche et le plus large possible pour répondre à ses propres besoins, mais également à ceux éventuels d’autres opérateurs industriels et de gestionnaires de services urbains publics.

OpenWize, le nom donné à cette couche protocolaire, a été développé pour être le plus agnostique possible par rapport au matériel.

Son architecture modulaire sous FreeRTOS permet cependant d’envisager un portage rapide vers une cible matériel de son choix. En adoptant une démarche open source, vise à enrichir l’écosystème de membres de l’alliance Wize et apportera son soutien à tous les contributeurs qui souhaiteraient s’investir dans les améliorations fonctionnelles ou le portage d’OpenWize.

OpenWize est disponible en téléchargement, mais peut-être également être intégré dans un module hardware open source Wize’Up développé par la société Alciom (www.alciom.com). Dans cette dernière configuration, une société qui découvre la technologie Wize, possède tous les éléments pour accélérer son projet IOT. 

OpenWize est constitué d'un écosystème qui s'appuie sur deux repositories :
 * le repository OpenWize XXXX qui contient le stack en lui meme qui s'accompagne d'un example rapide de mise en oeuvre.
 * le repository OpenWize'Up YYYY qui contient l'instanciation du stack OpenWize sur la carte Wize'Up développée par un membre de l'Alliance Wize, la société ALCIOM.



Les cartes de développement disponibles pour utiliser OpenWize
==============================================================

Wize’up : un module Wize haute performances et ouvert, la solution idéale pour développer simplement un produit compatible avec les réseaux LPWAN très longue distance Wize.

.. . figure:: pics/wize-up_board.png
..   :align: center
  
..   La carte Wize'up


.. figure:: pics/wize-up_base-board.png
   :align: center

   La carte Wize'Up montée sur sa carte de base


Wize’Up est le fruit d’un appel à projet de l’alliance wize, dans lequel les sociétés Alciom et GRDF ont réunies leurs savoirs-faires et expertises pour offrir une solution clés en main d’accélération de projets IOT. Le concept est de fournir un module de communication équipé d’OpenWize, facilement intégrable dans un nouveau design hardware. Ce module s’interface facilement via une liaison UART par l’intermédiaire d’un ensemble de commandes AT. Pour plus d’information sur `Wize’Up <https://www.alciom.com/nos-métiers/produits/wizeup/>`_


Pour plus d'information sur la mise en oeuvre du stack OpenWize sur la carte Wize'Up : (rajouter lien car l'index du readTheDocs de OpenWize'Up)


Vous trouverez également au paragraphe **add_link** un example simple d'utilisation d'OpenWize sur une carte de démonstration STM Nucléo L476RG qui ne possède pas de transceiver RF (la trame sort sur une UART de la carte).


Quels réseaux 169 MHz utiliser avec la technologie Wize
=======================================================

Wize est une technologie utilisable en mode « dédié » et en mode « opéré ».

En mode « dédié », un client décide d’installer sa propre infrastructure de concentrateurs Wize et s’enregistre en tant qu’opérateur auprès de l’alliance wize. Si vous développez un produit pour un client de ce type, votre client votre communiquera toutes les informations nécessaires pour l’enregistrement de votre produit sur son réseau.

En mode « opéré », un client décide d’utiliser le réseau d’un opérateur existant afin de faire remonter les données de ses capteurs vers son système d’information. Si vous développez un produit pour un client de ce type, votre client doit vous mettre en relation avec l’opérateur wize qu’il a sélectionné afin que vous puissiez enregistrer votre produit sur son réseau. La liste des opérateurs wize ainsi que les points de contacts associés sont disponibles sur le site de l’alliance.

A noter qu’à partir de la version 1.2, pour faciliter le développement de produit wize, l’alliance wize a introduit le concept Wiz’One qui permet de faire transiter ponctuellement des données sur tous les réseaux wize compatibles 1.2 à des fins de tests unitaires ou d’intégration. Le support de Wiz’One est dépendant de l’opérateur et peut-être associé à certaines restrictions en terme de fonctionnalités.


Pour aller plus loin dans votre développement
=============================================


Besoin d'aider ?
Vous pouvez contacter ....


Guide pour contribuer à OpenWize.
=================================

Les contributions à OpenWize – Bugs fix, ajout de nouvel fonctionnalités, ajout de documentation, ajouts d’exemples – sont les bienvenus. Ces contributions doivent s’effectuer sous la forme de Github Pull Request.

Prés-requis pour contribuer : (:ref:`Contribuer<Contributing>`)
 * Contribution sous la même licence que celle d’OpenWize.
 * Votre code doit être conforme au style de développement d’OpenWize (:ref:`Guide de l'écriture du code<Coding Style Guide>`)
 * Votre code doit être conforme au style de documentation d’OpenWize (:ref:`Guide de documentation<Documentation Guide>`)
 * Votre code doit être suffisamment documenter pour que sa structure soit facilement compréhensible.
 * Votre code est-il accompagné d’une documentation ou d’exemples qui permettent d’en comprendre facilement l’utilisation.
 * Les commentaires de votre code et votre documentation sont-ils écrits en anglais.


..
   User Guide de l'écriture du code
   --------------------------------
   TBD

   Si vous n’êtes pas sûr de l’ensemble de ces points, n’hésitez pas à ouvrir un issue sur le repository d'OpenWize pour nous poser vos questions.

.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Alciom` : https://www.alciom.com/en/home
.. _`Wize’Up`: https://www.alciom.com/en/our-trades/products/wizeup

