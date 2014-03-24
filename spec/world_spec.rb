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

def locations_from(range)
  range.each do |x|
    range.each do |y|
      range.each do |z|
        yield location(x, y, z)
      end
    end
  end
end

def build_block(loc)
  number = loc[:x] + loc[:y] + loc[:z]
  block = Redpile::Block.new
  block[:power] = number % 15
  block[:location] = loc
  block[:material] = Redpile::Material.symbols[number % Redpile::Material.symbols.count]
  block
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
  it 'stores and reads from blocks in a world' do
    single_section_world do |world_ptr|
      locations_from(0..15) do |loc|
        block = build_block(loc)
        Redpile.world_add_block(world_ptr, block)

        block_ptr = Redpile.world_get_block(world_ptr, loc)
        found_block = Redpile::Block.new(block_ptr)
        found_block[:power].should == block[:power]
        found_block[:location][:x].should == loc[:x]
        found_block[:location][:y].should == loc[:y]
        found_block[:location][:z].should == loc[:z]
        found_block[:material].should == block[:material]
      end
    end
  end
end

