require 'spec_helper'
include Helpers

describe 'Repeater' do
  ['wire', 'conductor'].each do |material|
    it "passes power to a #{material}" do
      run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        'NODE 0,0,1 REPEATER direction:SOUTH state:0',
        "NODE 0,0,2 #{material}",
        'TICK 2'
      ).should =~ /\(0,0,2\) FIELD power:15/
    end
  end

  (1..4).each do |delay|
    it "delays the propigation of power by #{delay} tick(s)" do
      result1 = run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        "NODE 0,0,1 REPEATER direction:SOUTH state:#{delay - 1}",
        'NODE 0,0,2 WIRE',
        "TICK #{delay}",
        'NODE 0,0,2'
      )
      contains_node?(result1, 0, 0, 2, 'WIRE', power: 0)

      result2 = run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        "NODE 0,0,1 REPEATER direction:SOUTH state:#{delay - 1}",
        'NODE 0,0,2 WIRE',
        "TICK #{delay}",
        'TICK',
        'NODE 0,0,2'
      )
      contains_node?(result2, 0, 0, 2, 'WIRE', power: 15)
    end
  end

  it 'is blocked from being powered by a repeater on the left' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE -1,0,2 SWITCH direction:UP state:1',
      'NODE 0,0,1 REPEATER direction:SOUTH state:0',
      'NODE 0,0,2 REPEATER direction:EAST state:0',
      'NODE 1,0,2 WIRE',
      'TICK 3',
      'NODE 1,0,2'
    )
    contains_node?(result, 1, 0, 2, 'WIRE', power: 0)
  end

  it 'is blocked from being powered by a repeater on the right' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 1,0,2 SWITCH direction:UP state:1',
      'NODE 0,0,1 REPEATER direction:SOUTH state:0',
      'NODE 0,0,2 REPEATER direction:WEST state:0',
      'NODE -1,0,2 WIRE',
      'TICK 3',
      'NODE -1,0,2'
    )
    contains_node?(result, -1, 0, 2, 'WIRE', power: 0)
  end
end
