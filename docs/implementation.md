Implementation
==============

Block Storage
-------------

**Few block changes**

Blocks are stored in a hashmap with each containing an array of pointers to adjacent blocks.  Every time a new block is inserted or removed, the hashmap is searched for adjacent blocks which are also updated.  This results in slow inserts but relatively fast 'random access' and instance access to adjacent blocks.

**Many blocks change**

Blocks are stored in a hashmap with no special access rules.

