Implementation
==============

**NOTE: This is horribly out of date, I'll be updating it once code churn settles down.**

Block Storage
-------------

Storage at its core is a series of [doubly linked lists](http://en.wikipedia.org/wiki/Linked_list#Doubly_linked_list) that store different block types.
In addition, each node stores pointers to all adjacent (in 3d space) nodes making a [adjacency list](http://en.wikipedia.org/wiki/Adjacency_list).
Finally, a [hashmap](http://en.wikipedia.org/wiki/Hash_table) is used for fast lookups when running `GET` and `SET` commands.

### Performance

* Traversal: O(n)
* Insertion: O(1)
* Removal: O(1)
* Lookup: O(1)
* Adjacent: O(1)

### Memory

*The numbers listed here were calculated on a 32 bit system*

* Total overhead: 84 bytes/block
  * List: 64 bytes/block
  * Hashmap: 20 bytes/block

The hashmap resizes in powers of two so there will aways be extra overhead.


Redstone Ticks
--------------

**Note:** Redstone ticks are under active development.  This document may be slightly out of date with the actual implementation.

When performing ticks there are 3 types of blocks that Redpile cares about:

* Boundaries - Ticks start and stop at these blocks
* Powerable - These blocks propagate power
* Unpowerable - These blocks stop the propagation of power

In addition, individual block types have [their own behavior](blocks.md) when powered.
These blocks are dealt with over the course of 4 steps in the tick.

### 1 - Search

Redpile visits every 'boundary' block in the map and calls a function to have it propagate out power.
That function in turn, calls more functions that spread outwards from the block.
As each of these functions run, they record any changes that should be made to the map as RUP (Redpile Update Programming) commands.

In its current state, this step is rather inefficient in terms of searching as it hits multiple blocks twice.
Eventually it will be replaced with [Dijkstra's algorithm](http://en.wikipedia.org/wiki/Dijkstra's_algorithm) to speed up searching.

### 2 - Aggregation

All RUP commands generated in step one are merged and grouped.
Any duplicate `POWER` commands are combined and `SWAP` commands are moved to after any `POWER` or `STATE` commands.
This step should go away once the search algorithm for step one is improved.

### 3 - Update

All RUP commands are executed in the order determined by step two.
This step is when all the command printing happens.

### 4 - Cleanup

Redpile visits every 'boundry' and 'powerable' block.  If they weren't updated their power is reset to zero.


Block Loading
-------------

Redpile implements 'lazy loading' of blocks in the world.
When a running tick can't find a specific block, it will call a function to populate that block.
Currently that function sets the block to `AIR` and returns.
In the future, this could be used to lazily load blocks from Minecraft maps as needed.

Benchmarks
----------

Redpile benchmarks are simple, they:

1. Inserts random blocks
2. Fetch previously inserted blocks
3. Run a couple redstone ticks
4. Remove all blocks

