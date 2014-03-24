require 'ffi'
require 'support/ffi_monkeypatch'
require 'support/redpile'

describe Redpile::Instruction do
  Redpile::COMMANDS.each do |key, val|
    it "parses the command '#{val}'" do
      FFI::MemoryPointer.new(:int) do |command|
        result = Redpile.command_parse(val, command)
        result.should == 0
        command.read_enum(Redpile::Command).should == key
      end
    end
  end

  it 'fails to parse an invaid command' do
    FFI::MemoryPointer.new(:int) do |command|
      result = Redpile.command_parse('BADCMD', command)
      result.should == -1
    end
  end

  Redpile::COMMANDS.each do |key, val|
    next if key == :cmd_tick

    it "parses the #{val} instruction" do
      FFI::MemoryPointer.new(Redpile::Instruction) do |inst_ptr|
        result = Redpile.instruction_parse("#{val} 5 4 3", inst_ptr)
        instruction = Redpile::Instruction.new(inst_ptr)
        result.should == 0
        instruction[:cmd].should == key
        instruction[:target][:x].should == 5
        instruction[:target][:y].should == 4
        instruction[:target][:z].should == 3
      end
    end
  end

  it 'parses the TICK instruction' do
    FFI::MemoryPointer.new(Redpile::Instruction) do |inst_ptr|
      result = Redpile.instruction_parse('TICK', inst_ptr)
      instruction = Redpile::Instruction.new(inst_ptr)
      result.should == 0
      instruction[:cmd].should == :cmd_tick
    end
  end

  it 'fails to parse an invalid instruction' do
    FFI::MemoryPointer.new(Redpile::Instruction) do |inst_ptr|
      result = Redpile.instruction_parse('BADINST', inst_ptr)
      instruction = Redpile::Instruction.new(inst_ptr)
      result.should == -1
    end
  end

  it 'fails to parse an missing parameter' do
    FFI::MemoryPointer.new(Redpile::Instruction) do |inst_ptr|
      result = Redpile.instruction_parse('ON 5 4', inst_ptr)
      instruction = Redpile::Instruction.new(inst_ptr)
      result.should == -1
    end
  end

  it 'fails to parse an non number parameter' do
    FFI::MemoryPointer.new(Redpile::Instruction) do |inst_ptr|
      result = Redpile.instruction_parse('ON 5 4 a', inst_ptr)
      instruction = Redpile::Instruction.new(inst_ptr)
      result.should == -1
    end
  end
end
