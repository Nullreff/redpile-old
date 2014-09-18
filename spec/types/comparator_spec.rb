require 'spec_helper'
include Helpers

describe 'Comparator' do
  ['wire', 'conductor'].each do |material|
    it "passes power to a #{material}" do
      run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        'NODE 0,0,1 COMPARATOR direction:SOUTH state:0',
        "NODE 0,0,2 #{material}",
        'TICK 2'
      ).should =~ /\(0,0,2\) FIELD power:15/
    end
  end

  it 'allows power through if side power is less than the input' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 WIRE',
      'NODE 1,0,3 SWITCH direction:UP state:1',
      'NODE 0,0,3 COMPARATOR direction:WEST state:0',
      'NODE -1,0,3 WIRE',
      'TICK 2',
      'NODE -1,0,3'
    )
    contains_node?(result, -1, 0, 3, 'WIRE', power: 15)
  end

  it 'does not allow power through if side power is greater than the input' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 2,0,1 SWITCH direction:UP state:1',
      'NODE 1,0,1 WIRE',
      'NODE 0,0,1 COMPARATOR direction:WEST state:0',
      'NODE -1,0,1 WIRE',
      'TICK 2',
      'NODE -1,0,1'
    )
    contains_node?(result, -1, 0, 1, 'WIRE', power: 0)
  end
end

