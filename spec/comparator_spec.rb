require 'spec_helper'
include Helpers

describe 'Comparator' do
  it 'allows power through if side power is less than the input' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 WIRE',
      'SET 1 0 3 TORCH UP',
      'SET 0 0 3 COMPARATOR WEST 0',
      'SET -1 0 3 WIRE',
      'TICK 2',
      'GET -1 0 3'
    ).should =~ /\(-1,0,3\) 15 WIRE/m
  end

  it 'does not allow power through if side power is greater than the input' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 2 0 1 TORCH UP',
      'SET 1 0 1 WIRE',
      'SET 0 0 1 COMPARATOR WEST 0',
      'SET -1 0 1 WIRE',
      'TICK 2',
      'GET -1 0 1'
    ).should =~ /\(-1,0,1\) 0 WIRE/m
  end
end

