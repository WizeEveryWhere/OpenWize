.. *****************************************************************************

************************
Welcome to OpenWize !
************************

What is Wize ?
=================

Launched in March 2017 by GRDF, Suez and Sagemcom, Wize is an open protocol whose principle is based on low-power, low-bitrate (between 1200 and 6400 bits/s) and long-range remote communication.

The Wize protocol specifications are maintained within the Wize Alliance, which is a non-profit association that promotes the adoption of this protocol within IOT networks in the world of industry and urban utilities. In 2021, the Wize Alliance will have about 40 members.

Wize uses radio waves on an open and royalty-free 169 MHz frequency in Europe to ensure the best possible penetration for collecting telemetry data in the Internet of Things domain.

All the documentation of the protocol is freely available on the `Wize Alliance website <https:\\www.wize-alliance.com>`_, as well as all the information needed to become a member in a few clicks.

Why a 169 MHz stack in open source ?
==========================================

GRDF started its work on the 169 MHZ frequency in 2010 as part of its remote reading of gas smart meters project. The challenge for GRDF was to set up a radio infrastructure on an open and free frequency band in Europe, in order to enable the remote reading of gas smart meters for its 11 million customers, based on :

 * Reliable technology allowing it to guarantee a daily reading rate of at least 95%,
 * A 20-year life span for battery-operated equipment,
 * A bidirectional communication allowing the sending of orders and the remote downloading of firmware inside its sensors,
 * Safe and secure end-to-end radio communication.

As no LPWAN at that time (and it is still the case in 2022) could meet all these constraints, GRDF decided to develop its own radio communication protocol, based on the W-MBUS mode N2 standard of the European technical committee 294, by integrating the functionalities that were missing at that time, i.e. bidirectionality and communication security mechanisms.

With more than 10 million meters deployed by end-2022 and in view of the excellent performance obtained with the technology implemented, GRDF has decided to participate in the promotion of the wize alliance by designing and developing a Wize protocol stack that is 100% compatible with the alliance's 1.2 specifications and to publish this development in open source mode under the MIT license. 

With this publication, GRDF aims to accelerate the adoption of this technology by as many people as possible and to facilitate the design and marketing of the richest and widest possible ecosystem of Wize-compatible IOT sensors to meet its own needs, as well as those of other industrial operators and public urban service managers.

OpenWize, the name given to this protocol layer, was developed to be as hardware agnostic as possible.

Its modular architecture under FreeRTOS allows however to consider a quick port to a hardware target of one's choice. By adopting an open source approach, Wize aims to enrich the ecosystem of members of the Wize alliance and will support all contributors who wish to invest in functional improvements or porting of OpenWize.

OpenWize is available for download, but can also be integrated into an open source hardware module Wize'Up developed by the company Alciom (www.alciom.com). In this last configuration, a company that discovers the Wize technology, has all the elements to accelerate its IOT project.

OpenWize consists of an ecosystem based on three repositories:
 * the `OpenWize`_ repository which contains the stack itself and a quick implementation example.
 * the `OpenWize'Up`_ repository which contains the instantiation of the OpenWize stack on the Wize'Up card developed by a member of the Wize Alliance, the company ALCIOM.
 * the `banc_lib`_ repository which contains a test tool for Wize protocol with SmartBrick device



The development cards available for using OpenWize
==============================================================

Wize'up: a high performance and open Wize module, the ideal solution to simply develop a product compatible with Wize LPWANs.

.. . figure:: pics/wize-up_board.png
..   :align: center
  
..   La carte Wize'up


.. figure:: pics/wize-up_base-board.png
   :align: center

   The Wize'Up card mounted on its base card


Wize'Up is the result of a call for projects from the wize alliance, in which Alciom and GRDF have combined their skills and expertise to offer a turnkey solution for accelerating IOT projects. The concept is to provide a communication module equipped with OpenWize, easily integrated into a new hardware design. This module interfaces easily via a UART link through a set of AT commands. For more information on `Wize’Up <https://www.alciom.com/nos-métiers/produits/wizeup/>`_


For more information on the implementation of the OpenWize stack on the Wize'Up map : (rajouter lien car l'index du readTheDocs de OpenWize'Up)


You will also find in the :ref:`Demo application` paragraph a simple example of using OpenWize on a Nucléo L476RG STM demonstration board that does not have an RF transceiver (the frame is output on a UART on the board).


Which 169 MHz networks to use with Wize technology ?
=======================================================

Wize is a technology that can be used in both "dedicated" and "operated" modes.

In "dedicated" mode, a customer decides to install its own infrastructure of Wize hubs and registers as an operator with the wize alliance. If you develop a product for such a customer, your customer will provide you with all the necessary information to register your product on their network.

In "operated" mode, a customer decides to use the network of an existing operator to send data from its sensors to its information system. If you develop a product for a customer of this type, your customer must put you in touch with the wize operator he has selected so that you can register your product on his network. The list of wize operators as well as the associated contact points are available on the alliance's website.

Note that from version 1.2, to facilitate the development of wize products, the wize alliance has introduced the Wiz'One concept which allows data to be transmitted on all 1.2 compatible wize networks for unit testing or integration purposes. Wiz'One support is operator dependent and may be associated with certain restrictions in terms of functionality.


To go further in your development
=============================================


Need to help?
You can contact Marvin JEANNOT


Guide to contributing to OpenWize.
=================================

Contributions to OpenWize - bug fixes, adding new features, adding documentation, adding examples - are welcome. These contributions should be made in the form of a Github Pull Request.

Prerequisites to contribute: (:ref:`Contribuer<Contributing>`)
 * Contributed under the same license as OpenWize.
 * Your code must conform to the OpenWize development style (:ref:`Guide de l'écriture du code<Coding Style Guide>`)
 * Your code must conform to the OpenWize documentation style (:ref:`Documentation Guide<Documentation Guide>`)
 * Your code must be sufficiently documented so that its structure is easily understood.
 * Is your code accompanied by documentation or examples that make it easy to understand its use?
 * Are your code comments and documentation written in English ?


..
   User Guide to Writing Code
   --------------------------------
   TBD

   If you are unsure about any of these points, feel free to open an issue on the OpenWize repository to ask us your questions.

.. *****************************************************************************
.. references
.. _`STM32CubeIDE`: https://www.st.com/en/development-tools/stm32cubeide.html#get-software
.. _`Alciom` : https://www.alciom.com/en/home
.. _`Wize’Up`: https://www.alciom.com/en/our-trades/products/wizeup
.. _`OpenWize`: https://github.com/WizeEveryWhere/OpenWize
.. _`OpenWize'Up`: https://github.com/WizeEveryWhere/OpenWize-Up
.. _`banc_lib`: https://github.com/WizeEveryWhere/banc_lib
