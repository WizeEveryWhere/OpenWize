*WizeCore* sub directory
------------------------

``proto``
   *proto* implement all necessary to treat the link and presentation layer from
   the Wize protocol, that is :

   - build/extract the Link and Presentation Layers information
   - cipher/un-cipher the application message
   - compute and check the authentication hash 
   - compute and check the confidentiality hash
   - compute and check the CRC
   - compute and check the Reed-Solomon code and correct the frame when possible (Download layer only)

   *proto* depends on the "CRC_sw", "Crypto" and "ReedSolomon" libraries.


``net``
   *net* implement a kind of low-level driver to : 

   - drive the protocol in *proto*
   - provide an abstract layer to higher level, with function like "send(...)" or "recev(...)".
   - provide an interface to the phy driver implementation

   *net* depends on *proto* 

``mgr``
   *mgr* implement four managers :

   - *adm_mgr*, *inst_mgr* and *dwn_mgr* manage respectively the administration, installation and download sessions. Their role is mainly to ensure the session timing correctness.
   
   - *net_mgr* is a kind of high level driver for the *net* module. In particular, it manage the net+phy resources in thread-safe manner and ensure the timing correctness if the "send" and "listen" windows.

   *mgr* depends on *net*, *TimeEvt*, *Logger* and *FreeRTOS*.

``app``
   *app* implement application layer of the Wize protocol and provide an API to the rest of application firmware.

   - provide API to send DATA message and execute an installation (PING/PONG) session
   - open/close administration, installation and download sessions.
   - build/extract generic administration layer (aka DATA, COMMAND, RESPONSE)
   - build/extract installation layer (aka PING/PONG)
   - deal with downloaded firmware image (store, validate)

   *app* depends on *mgr*, *TimeEvt*, *ImgStorage*, *Parameters*, *Logger* and *FreeRTOS*.
