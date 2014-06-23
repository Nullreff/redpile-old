require 'spec_helper'
include Helpers

# More complex logic gate tests.  Taken from:
# http://minecraft.gamepedia.com/Tutorials/Basic_Logic_Gates

describe 'Logic' do
  context 'NOP' do
    def nop_gate(in1)
      run(
        "SET 0 0 0 SWITCH NORTH #{in1}",
        'SET 0 0 1 CONDUCTOR',
        'SET 0 0 2 WIRE',
        'TICK 2',
        'GET 0 0 2')
    end

    it('0 => 0') {nop_gate(0).should =~ unpowered(0, 0, 2)}
    it('1 => 1') {nop_gate(1).should =~ powered(0, 0, 2)}
  end

  context 'NOT' do
    def not_gate(in1)
      run(
        "SET 0 0 0 SWITCH NORTH #{in1}",
        'SET 0 0 1 CONDUCTOR',
        'SET 0 0 2 TORCH SOUTH',
        'SET 0 0 3 WIRE',
        'TICK 3',
        'GET 0 0 3')
    end

    it('0 => 1') {not_gate(0).should =~ powered(0, 0, 3)}
    it('1 => 0') {not_gate(1).should =~ unpowered(0, 0, 3)}
  end

  context 'AND' do
    def and_gate(in1, in2)
      run(
        "SET 0 0 0 SWITCH NORTH #{in1}",
        "SET 2 0 0 SWITCH NORTH #{in2}",
        'SET 0 0 1 CONDUCTOR',
        'SET 1 0 1 CONDUCTOR',
        'SET 2 0 1 CONDUCTOR',
        'SET 0 1 1 TORCH UP',
        'SET 1 1 1 WIRE',
        'SET 2 1 1 TORCH UP',
        'SET 1 0 2 TORCH SOUTH',
        'SET 1 0 3 WIRE',
        'TICK 4',
        'GET 1 0 3')
    end

    it('0 0 => 0') {and_gate(0, 0).should =~ unpowered(1, 0, 3)}
    it('0 1 => 0') {and_gate(0, 1).should =~ unpowered(1, 0, 3)}
    it('1 0 => 0') {and_gate(1, 0).should =~ unpowered(1, 0, 3)}
    it('1 1 => 1') {and_gate(1, 1).should =~   powered(1, 0, 3)}
  end

  context 'NAND' do
    def nand_gate(in1, in2)
      run(
        "SET 0 0 0 SWITCH NORTH #{in1}",
        "SET 2 0 0 SWITCH NORTH #{in2}",
        'SET 0 0 1 CONDUCTOR',
        'SET 1 0 1 CONDUCTOR',
        'SET 2 0 1 CONDUCTOR',
        'SET 0 1 1 TORCH UP',
        'SET 1 1 1 WIRE',
        'SET 2 1 1 TORCH UP',
        'SET 1 0 2 WIRE',
        'TICK 3',
        'GET 1 0 2')
    end

    it('0 0 => 1') {nand_gate(0, 0).should =~   powered(1, 0, 2)}
    it('0 1 => 1') {nand_gate(0, 1).should =~   powered(1, 0, 2)}
    it('1 0 => 1') {nand_gate(1, 0).should =~   powered(1, 0, 2)}
    it('1 1 => 0') {nand_gate(1, 1).should =~ unpowered(1, 0, 2)}
  end

  context 'OR' do
    def or_gate(in1, in2)
      run(
        "SET 0 0 0 SWITCH NORTH #{in1}",
        "SET 2 0 0 SWITCH NORTH #{in2}",
        'SET 0 0 1 CONDUCTOR',
        'SET 2 0 1 CONDUCTOR',
        'SET 0 0 2 REPEATER SOUTH 0',
        'SET 2 0 2 REPEATER SOUTH 0',
        'SET 0 0 3 WIRE',
        'SET 1 0 3 WIRE',
        'SET 2 0 3 WIRE',
        'SET 1 0 4 WIRE',
        'TICK 4',
        'GET 1 0 4')
    end

    it('0 0 => 0') {or_gate(0, 0).should =~ unpowered(1, 0, 4)}
    it('0 1 => 1') {or_gate(0, 1).should =~   powered(1, 0, 4)}
    it('1 0 => 1') {or_gate(1, 0).should =~   powered(1, 0, 4)}
    it('1 1 => 1') {or_gate(1, 1).should =~   powered(1, 0, 4)}
  end

  context 'NOR' do
    def nor_gate(in1, in2)
      run(
        "SET 0 0 0 SWITCH WEST #{in1}",
        'SET 1 0 0 CONDUCTOR',
        "SET 2 0 0 SWITCH EAST #{in2}",
        'SET 1 0 1 TORCH SOUTH',
        'SET 1 0 2 WIRE',
        'TICK 3',
        'GET 1 0 2')
    end

    it('0 0 => 1') {nor_gate(0, 0).should =~   powered(1, 0, 2)}
    it('0 1 => 0') {nor_gate(0, 1).should =~ unpowered(1, 0, 2)}
    it('1 0 => 0') {nor_gate(1, 0).should =~ unpowered(1, 0, 2)}
    it('1 1 => 0') {nor_gate(1, 1).should =~ unpowered(1, 0, 2)}
  end
end
