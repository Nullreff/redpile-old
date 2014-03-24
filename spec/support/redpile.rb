module Redpile
  extend FFI::Library
  ffi_lib 'build/libredpiletest.so'

  COMMANDS = {
    cmd_on: 'ON',
    cmd_off: 'OFF',
    cmd_toggle: 'TOGGLE',
    cmd_tick: 'TICK'
  }

  Command = enum(COMMANDS.keys)

  class Location < FFI::Struct
    layout :x, :long,
           :y, :long,
           :z, :long
  end

  class Instruction < FFI::Struct
    layout :cmd, Command,
           :target, Location
  end

  Material = enum(:wire)

  class Block <FFI::Struct
    layout :location, Location,
           :material, Material,
           :power, :int
  end

  class Section < FFI::Struct
    layout :location, Location,
           :blocks, :pointer
  end

  class World < FFI::Struct
    layout :count, :int,
           :sections, :pointer
  end

  attach_function :command_parse, [:string, :pointer], :int
  attach_function :instruction_parse, [:string, :pointer], :int

  attach_function :world_intialize, [World], :void
  attach_function :world_free, [World], :void
  attach_function :world_initialize_section, [World, Location.by_value], :void
  attach_function :world_add_block, [World, Block.by_value], :int
  attach_function :world_get_block, [World, Location.by_value], :pointer
end

