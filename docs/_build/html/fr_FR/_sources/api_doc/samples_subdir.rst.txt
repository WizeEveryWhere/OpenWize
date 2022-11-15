.. *****************************************************************************

*Samples* sub directory
-----------------------

``CRC_sw``
   *CRC_sw* library provides functions to compute and check the CRC16 (*Cyclic Redundancy Check*) 
   as specified by `Wize Lan Protocol Specifications`_ v1.2.

``Crypto``
   *Crypto* library provides interface functions to cipher, uncipher, compute 
   AES-CMAC and SHA256. This module relies on *tinycrypt* library located in 
   "third-party/libraries/Tinycrypt" directory (https://github.com/intel/tinycrypt.git).

``ReedSolomon``
   *ReedSolomon* library implement Reed-Solomon code correction algorithm 
   RS(255,223). This module is a port of the http://www.eccpage.com/rs.c

``ImgStorage``
   *ImgStorage* library provides an interface for storing downloaded firmware 
   block on physical memory.

``Parameters``
   *Parameters* library provides interfaces to set and get parameters. 

``TimeEvt``
   *TimeEvt* module provides event to the task at the required time.  

``Logger``
   *Logger* module provides a way to log out messages (info, warning, error, debug).
   

.. *****************************************************************************
.. references
.. _`Wize Lan Protocol Specifications`: https://www.wize-alliance.com/Downloads/Technical
