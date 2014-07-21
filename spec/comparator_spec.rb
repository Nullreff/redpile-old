require 'spec_helper'
include Helpers

describe 'Comparator' do
  ['wire', 'conductor'].each do |material|
    it "passes power to a #{material}" do
      run(
        'SET 0 0 0 SWITCH direction:UP state:1',
        'SET 0 0 1 COMPARATOR direction:SOUTH state:0',
        "SET 0 0 2 #{material}",
        'TICK 2'
      ).should =~ /\(0,0,2\) POWER 15/
    end
  end

  it 'allows power through if side power is less than the input' do
    run(
      'SET 0 0 0 SWITCH direction:UP state:1',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 WIRE',
      'SET 1 0 3 SWITCH direction:UP state:1',
      'SET 0 0 3 COMPARATOR direction:WEST state:0',
      'SET -1 0 3 WIRE',
      'TICK 2',
      'GET -1 0 3'
    ).should =~ /^\(-1,0,3\) WIRE 15$/
  end

  it 'does not allow power through if side power is greater than the input' do
    run(
      'SET 0 0 0 SWITCH direction:UP state:1',
      'SET 2 0 1 SWITCH direction:UP state:1',
      'SET 1 0 1 WIRE',
      'SET 0 0 1 COMPARATOR direction:WEST state:0',
      'SET -1 0 1 WIRE',
      'TICK 2',
      'GET -1 0 1'
    ).should =~ /^\(-1,0,1\) WIRE 0$/
  end
end

