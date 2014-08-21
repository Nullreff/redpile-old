Introduction
============

The world as Redpile sees it, is made up of 'nodes' that exist as a grid in three dimensional space.
When adding a node to the world, you assigned a type that determines how if functions.
Every type has a list of fields that it needs (direction, power, state, etc...).
Types also have a list of behaviors that determine how they interact with the world.
To communicate, nodes can pass messages between each other.
During a tick, behaviors take a list of received messages,
produce messages to send to other nodes and ask the system to make modifications to the world.
Once all nodes have had a chance to run their behaviors, world modifications are applied.
