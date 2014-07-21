Messages
========

Interface used for passing messages.

Commands
--------

### SEND

Syntax: `SEND delay target message` => `void`

Sends a message to another node

### READ

Syntax: `READ pattern` => `[(source,message)]`

Reads a series of message that match a specific pattern.

### SET

Syntax: `SET key value` => `void`

Sets a value on the current node

### GET

Syntax: `GET key` => `{value}`

Gets a value from the current node

Parameters
----------

### delay

The number of ticks to wait before a message should arrive at another node.
A delay of `0` will pass it on the same tick as the execution of the originating command.
The message will only be available during the tick it's specified to arrive on.
Specifying a negative delay should result in an error (though it would be cool to support time travel).

### target/source

An identifier used to determine the target/source of a command.
This is always relative to the node executing the command.  In Redpile, it is (x,y,z) coordinates.
However, any system with relative identifiers could in theory be used.

### message

A message compressed as a series of bit fields.
Should ideally contain minimal information or reference large immutable information stored elsewhere.

### pattern

A boolean statement that inspects the passed message, its source and the state of the current node.

### key

An identifier used to access a value from a node.
Internally it is tracked as a unique integer but could be mapped to a string.

### value

The value stored in a key.  Internally is tracked as an integer but could be adapted to store just about anything.  This can be done either via conversion to integer or having the integer act as a pointer to another area of memory.

