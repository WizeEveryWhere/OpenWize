

Appendix
========

Modifying the default parameters
--------------------------------

(See "HowTo.md" in "tools/scripts/gen_param directory").

In "demo/Nucleo-L476/app/cfg, open and modify as you need the "DefaultParams.xml"
file. Then, regenerate the default tables as explained in the following.

Prerequisite : 
The "xmlstarlet" tool has to be installed. (sudo apt-get install xmlstarlet)
The "xmlmerge" tool has to be installed. (sudo apt-get install gwenhywfar-tools)

Setup the "gen_table.sh" script path in environment PATH variable:

.. code-block:: bash

   cd OpenWize
   export PATH=$PATH:$(pwd)/tools/scripts/gen_param
   cd demo/Nucleo-L476/app


Merge the two xml files :

.. code-block:: bash

   xmlmerge ./cfg/DefaultParams.xml ./cfg/DefaultRestr.xml -o ./cfg/MergedParam.xml

Then, call "gen_table.sh" :

.. code-block:: bash

   gen_table.sh --in ./cfg/MergedParam.xml --out .


.. rubric::  It will generate 3 files in "gen" directory:

::

   gen
   ├──parameters_cfg.h     : access and restriction table declaration
   ├──parameters_cfg.c     : access and restriction table definition
   └──parameters_default.c : define the tables default content

In these files : 

- *a_ParamAccess[]* : gives the parameters access right. 

This table describe for each parameter, its id, local access rights, remote 
access rights, update time, referenced, size, offset and restriction id 
(see the software documentation for details).

- *a_ParamRestr[]* : gives the restrictions on parameters values (if any) 

This table describe for each restriction, its id, type (modulo, range, enum), 
the element size (8, 16, 32, 64 bits), the number of restriction element 
(modulo: 1; range: 2; enum: n elements), address of the element table (see the 
software documentation for details).

- *a_ParamDefault[]* : gives the parameters defaults values




