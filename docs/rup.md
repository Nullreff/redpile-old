Redpile Update Programming
==========================

Interface used for logic implementations.

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

### QUERY

Syntax: `QUERY target key` => `{value}`

Returns a value from another node

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

A boolean statement that inspects the passed message, its source and the state of the current block.

### key

An identifier used to access a value from a node.
Internally it is tracked as a unique integer but could be mapped to a string.

### value

The value stored in a key.  Internally is tracked as an integer but could be adapted to store just about anything.  This can be done either via conversion to integer or having the integer act as a pointer to another area of memory.

Language
--------

In order to make Redpile easier to program, a higher level language should be implemented.
This language will be compiled down to a series of RUP commands.

* Use pattern matching on messages sent.
* Patterns are processed in order and the first matching one is run.
* Should (ideally) be a non-turning complete language.

The following is using ruby syntax but a custom compiled language would probably be faster.

```ruby
# Syntax of: name, [values]
message_field :command, [:power, :break]
message_field :power, (0..15)

block :wire do
    # Fields stored directly on this block
    field :power

    # If any message matching the condition are found,
    # calls the block with them.  All others are discared.
    match('message.command == :break') do |cmds|
        self.delete
    end

    # Possibly include duplicate functionality via mixins?
    match('message.command == :power && message.power > self.power') do |cmds|
        self.power = cmds.map(&:power).max
        for_blocks(:north, :south, :east, :west) do |block|
            block.send(:power, self.power - 1)
        end
    end

    # If nothing is matched, the default method is run.
    default do
        self.power = 0
    end
end
```

