require 'spec_helper'
include Helpers

describe 'Torch' do
  MAX_RANGE = 15
  (1..MAX_RANGE).each do |range|
    it "propigates power #{range} blocks" do
      run(
        'NODE 0,0,0 TORCH direction:UP',
        "NODE 0,0,1..#{range} WIRE",
        'TICK 2'
      ).should =~ /^0,0,#{range} FIELD power:#{16 - range}$/
    end
  end

  it "stops propigating power after #{MAX_RANGE} blocks" do
    end_block = MAX_RANGE + 1
    result = run(
      'NODE 0,0,0 TORCH direction:UP',
      "NODE 0,0,1..#{end_block} WIRE",
      'TICK 2',
      "NODE 0,0,#{end_block}"
    )
    contains_node?(result, 0, 0, end_block, 'WIRE', power: 0)
  end

  it 'takes power from the closer torch' do
    run(
      'NODE 0,0,-3 TORCH direction:NORTH',
      'NODE 0,0,-2 WIRE',
      'NODE 0,0,-1 WIRE',
      'NODE 0,0,0 WIRE',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 TORCH direction:NORTH',
      'TICK 2'
    ).should =~ /0,0,0 FIELD power:14\n/
  end

  it 'turns a torch off with power' do
    result = run(
      'NODE 0,0,1 WIRE',
      'NODE 0,0,0 TORCH direction:SOUTH',
      'NODE 0,0,-1 WIRE',
      'NODE 0,0,-2 SWITCH direction:UP state:1',
      'TICK 2',
      'NODE 0,0,1'
    )
    contains_node?(result, 0, 0, 1, 'WIRE', power: 0)
  end

  it 'passes power up through a conductor' do
    result = run(
      'NODE 0,0,0 TORCH direction:UP',
      'NODE 0,1,0 CONDUCTOR',
      'NODE 0,2,0 WIRE',
      'TICK 2'
    )
    result.should =~ /0,1,0 FIELD power:15/
    result.should =~ /0,2,0 FIELD power:15/
  end
end
