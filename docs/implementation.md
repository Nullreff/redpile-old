Implementation
==============

Block Storage
-------------

Storage at its core is a series of [doubly linked lists](http://en.wikipedia.org/wiki/Linked_list#Doubly_linked_list) that store different block types.
In addition, each node stores pointers to all adjacent (in 3d space) nodes making a [adjacency list](http://en.wikipedia.org/wiki/Adjacency_list).
Finally, a [hashmap](http://en.wikipedia.org/wiki/Hash_table) is used for fast lookups when running `GET` and `SET` commands.

* Traversal: O(n)
* Insertion: O(1)
* Removal: O(1)
* Lookup: O(1)
* Adjacent: O(1)

Redstone Ticks
--------------

There is a lot of code churn going on right now.  Once it settles down this section will be updated.

Block Loading
-------------

Redpile implements 'lazy loading' of blocks in the world.
When a running tick can't find a specific block, it will call a function to populate that block.
Currently that function sets the block to `AIR` and returns.
In the future, this could be used to lazily load blocks from Minecraft maps as needed.

