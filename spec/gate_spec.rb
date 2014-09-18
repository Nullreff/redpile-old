require 'spec_helper'
include Helpers

# More complex logic gate tests.  Taken from:
# http://minecraft.gamepedia.com/Tutorials/Basic_Logic_Gates

describe 'Logic' do
  context 'NOP' do
    def nop_gate(in1)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        'NODE 0 0 1 CONDUCTOR',
        'NODE 0 0 2 WIRE',
        'TICK',
        'NODE 0 0 2')
    end

    it('0 => 0') {nop_gate(0).should =~ unpowered(0, 0, 2)}
    it('1 => 1') {nop_gate(1).should =~ powered(0, 0, 2)}
  end

  context 'NOT' do
    def not_gate(in1)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        'NODE 0 0 1 CONDUCTOR',
        'NODE 0 0 2 TORCH direction:SOUTH',
        'NODE 0 0 3 WIRE',
        'TICK 2',
        'NODE 0 0 3')
    end

    it('0 => 1') {not_gate(0).should =~ powered(0, 0, 3)}
    it('1 => 0') {not_gate(1).should =~ unpowered(0, 0, 3)}
  end

  context 'AND' do
    def and_gate(in1, in2)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        "NODE 2 0 0 SWITCH direction:NORTH state:#{in2}",
        'NODE 0 0 1 CONDUCTOR',
        'NODE 1 0 1 CONDUCTOR',
        'NODE 2 0 1 CONDUCTOR',
        'NODE 0 1 1 TORCH direction:UP',
        'NODE 1 1 1 WIRE',
        'NODE 2 1 1 TORCH direction:UP',
        'NODE 1 0 2 TORCH direction:SOUTH',
        'NODE 1 0 3 WIRE',
        'TICK 3',
        'NODE 1 0 3')
    end

    it('0 0 => 0') {and_gate(0, 0).should =~ unpowered(1, 0, 3)}
    it('0 1 => 0') {and_gate(0, 1).should =~ unpowered(1, 0, 3)}
    it('1 0 => 0') {and_gate(1, 0).should =~ unpowered(1, 0, 3)}
    it('1 1 => 1') {and_gate(1, 1).should =~   powered(1, 0, 3)}
  end

  context 'NAND' do
    def nand_gate(in1, in2)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        "NODE 2 0 0 SWITCH direction:NORTH state:#{in2}",
        'NODE 0 0 1 CONDUCTOR',
        'NODE 1 0 1 CONDUCTOR',
        'NODE 2 0 1 CONDUCTOR',
        'NODE 0 1 1 TORCH direction:UP',
        'NODE 1 1 1 WIRE',
        'NODE 2 1 1 TORCH direction:UP',
        'NODE 1 0 2 WIRE',
        'TICK 2',
        'NODE 1 0 2')
    end

    it('0 0 => 1') {nand_gate(0, 0).should =~   powered(1, 0, 2)}
    it('0 1 => 1') {nand_gate(0, 1).should =~   powered(1, 0, 2)}
    it('1 0 => 1') {nand_gate(1, 0).should =~   powered(1, 0, 2)}
    it('1 1 => 0') {nand_gate(1, 1).should =~ unpowered(1, 0, 2)}
  end

  context 'OR' do
    def or_gate(in1, in2)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        "NODE 2 0 0 SWITCH direction:NORTH state:#{in2}",
        'NODE 0 0 1 CONDUCTOR',
        'NODE 2 0 1 CONDUCTOR',
        'NODE 0 0 2 REPEATER direction:SOUTH state:0',
        'NODE 2 0 2 REPEATER direction:SOUTH state:0',
        'NODE 0 0 3 WIRE',
        'NODE 1 0 3 WIRE',
        'NODE 2 0 3 WIRE',
        'NODE 1 0 4 WIRE',
        'TICK 3',
        'NODE 1 0 4')
    end

    it('0 0 => 0') {or_gate(0, 0).should =~ unpowered(1, 0, 4)}
    it('0 1 => 1') {or_gate(0, 1).should =~   powered(1, 0, 4)}
    it('1 0 => 1') {or_gate(1, 0).should =~   powered(1, 0, 4)}
    it('1 1 => 1') {or_gate(1, 1).should =~   powered(1, 0, 4)}
  end

  context 'NOR' do
    def nor_gate(in1, in2)
      run(
        "NODE 0 0 0 SWITCH direction:WEST state:#{in1}",
        'NODE 1 0 0 CONDUCTOR',
        "NODE 2 0 0 SWITCH direction:EAST state:#{in2}",
        'NODE 1 0 1 TORCH direction:SOUTH',
        'NODE 1 0 2 WIRE',
        'TICK 2',
        'NODE 1 0 2')
    end

    it('0 0 => 1') {nor_gate(0, 0).should =~   powered(1, 0, 2)}
    it('0 1 => 0') {nor_gate(0, 1).should =~ unpowered(1, 0, 2)}
    it('1 0 => 0') {nor_gate(1, 0).should =~ unpowered(1, 0, 2)}
    it('1 1 => 0') {nor_gate(1, 1).should =~ unpowered(1, 0, 2)}
  end

  context 'XOR' do
    def xor_gate(in1, in2)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        "NODE 2 0 0 SWITCH direction:NORTH state:#{in2}",

        'NODE 0 0 1 CONDUCTOR',
        'NODE 1 0 1 CONDUCTOR',
        'NODE 2 0 1 CONDUCTOR',
        'NODE 0 1 1 TORCH direction:UP',
        'NODE 1 1 1 WIRE',
        'NODE 2 1 1 TORCH direction:UP',

        'NODE 0 0 2 TORCH direction:SOUTH',
        'NODE 1 0 2 CONDUCTOR',
        'NODE 2 0 2 TORCH direction:SOUTH',
        'NODE 1 1 2 WIRE',

        'NODE 0 0 3 WIRE',
        'NODE 1 0 3 TORCH direction:SOUTH',
        'NODE 2 0 3 WIRE',

        'NODE 0 0 4 CONDUCTOR',
        'NODE 2 0 4 CONDUCTOR',
        'NODE 0 1 4 WIRE',
        'NODE 2 1 4 WIRE',

        'NODE 0 0 5 TORCH direction:SOUTH',
        'NODE 1 0 5 WIRE',
        'NODE 2 0 5 TORCH direction:SOUTH',

        'TICK 4',
        'NODE 1 0 5')
    end

    it('0 0 => 0') {xor_gate(0, 0).should =~ unpowered(1, 0, 5)}
    it('0 1 => 1') {xor_gate(0, 1).should =~   powered(1, 0, 5)}
    it('1 0 => 1') {xor_gate(1, 0).should =~   powered(1, 0, 5)}
    it('1 1 => 0') {xor_gate(1, 1).should =~ unpowered(1, 0, 5)}
  end

  context 'XNOR' do
    def xnor_gate(in1, in2)
      run(
        "NODE 0 0 0 SWITCH direction:NORTH state:#{in1}",
        "NODE 2 0 0 SWITCH direction:NORTH state:#{in2}",

        'NODE 0 0 1 CONDUCTOR',
        'NODE 1 0 1 CONDUCTOR',
        'NODE 2 0 1 CONDUCTOR',
        'NODE 0 1 1 TORCH direction:UP',
        'NODE 1 1 1 WIRE',
        'NODE 2 1 1 TORCH direction:UP',

        'NODE 0 0 2 TORCH direction:SOUTH',
        'NODE 1 0 2 CONDUCTOR',
        'NODE 2 0 2 TORCH direction:SOUTH',
        'NODE 1 1 2 WIRE',

        'NODE 0 0 3 WIRE',
        'NODE 1 0 3 TORCH direction:SOUTH',
        'NODE 2 0 3 WIRE',

        'NODE 0 0 4 CONDUCTOR',
        'NODE 2 0 4 CONDUCTOR',
        'NODE 0 1 4 WIRE',
        'NODE 2 1 4 WIRE',

        'NODE 0 0 5 TORCH direction:SOUTH',
        'NODE 1 0 5 WIRE',
        'NODE 2 0 5 TORCH direction:SOUTH',

        'NODE 1 0 6 CONDUCTOR',
        'NODE 1 1 6 WIRE',
        'NODE 2 0 6 TORCH direction:EAST',

        'NODE 2 0 7 WIRE',

        'TICK 5',
        'NODE 2 0 7')
    end

    it('0 0 => 1') {xnor_gate(0, 0).should =~   powered(2, 0, 7)}
    it('0 1 => 0') {xnor_gate(0, 1).should =~ unpowered(2, 0, 7)}
    it('1 0 => 0') {xnor_gate(1, 0).should =~ unpowered(2, 0, 7)}
    it('1 1 => 1') {xnor_gate(1, 1).should =~   powered(2, 0, 7)}
  end
end
