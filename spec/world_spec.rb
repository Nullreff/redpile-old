require 'ffi'
require 'support/ffi_monkeypatch'
require 'support/redpile'

def location(x, y, z)
  location = Redpile::Location.new
  location[:x] = x
  location[:y] = y
  location[:z] = z
  location
end

def single_section_world
  FFI::MemoryPointer.new(Redpile::World) do |world_ptr|
    Redpile.world_intialize(world_ptr)
    Redpile.world_initialize_section(world_ptr, location(0, 0, 0))
    yield world_ptr
    Redpile.world_free(world_ptr)
  end
end

describe Redpile::World do
  it 'stores a block in a world' do
    single_section_world do |world_ptr|
      block = Redpile::Block.new
      block[:power] = 5
      block[:location] = location(6, 7, 8)
      block[:material] = :wire
      Redpile.world_add_block(world_ptr, block)

      block_ptr = Redpile.world_get_block(world_ptr, location(6, 7, 8))
      block = Redpile::Block.new(block_ptr)
      block[:power].should == 5
      block[:location][:x].should == 6
      block[:location][:y].should == 7
      block[:location][:z].should == 8
      block[:material].should == :wire
    end
  end
end


# attach_function :world_intialize, [:pointer], :void
# attach_function :world_free, [:pointer], :void
# attach_function :world_initialize_section, [:pointer, Location], :void
# attach_function :world_get_block, [:pointer, Location], :pointer
