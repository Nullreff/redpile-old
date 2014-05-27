Implementation
==============

Block Storage
-------------

Storage at its core is a series of [doubly linked lists](http://en.wikipedia.org/wiki/Linked_list#Doubly_linked_list) that store different block types.  In addition, each node stores pointers to all adjacent (in 3d space) nodes making a [adjaceny list](http://en.wikipedia.org/wiki/Adjacency_list).  Finally, a [hashmap](http://en.wikipedia.org/wiki/Hash_table) is used for fast lookups when running `GET` and `SET` commands.

Performance is:

* Traversal: O(n)
* Insertion: O(1)
* Removal: O(1)
* Lookup: O(1)
* Adjacent: O(1)

