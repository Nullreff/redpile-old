require 'spec_helper'
include Helpers

describe 'Torch' do
  MAX_RANGE = 15
  (1..MAX_RANGE).each do |range|
    it "propigates power #{range} blocks" do
      run(
        'SET 0 0 0 TORCH UP',
        "SETR 0 0 1 0 0 #{range} WIRE",
        'TICK 2'
      ).should =~ /\(0,0,#{range}\) POWER #{16 - range}\n/
    end
  end

  it "stops propigating power after #{MAX_RANGE} blocks" do
    end_block = MAX_RANGE + 1
    run(
      'SET 0 0 0 TORCH UP',
      "SETR 0 0 1 0 0 #{end_block} WIRE",
      'TICK 2',
      "GET 0 0 #{end_block}"
    ).should =~ /^\(0,0,#{end_block}\) WIRE 0$/
  end

  it 'takes power from the closer torch' do
    run(
      'SET 0 0 -3 TORCH NORTH',
      'SET 0 0 -2 WIRE',
      'SET 0 0 -1 WIRE',
      'SET 0 0 0 WIRE',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 TORCH NORTH',
      'TICK 2'
    ).should =~ /\(0,0,0\) POWER 14\n/
  end

  it 'turns a torch off with power' do
    run(
      'SET 0 0 1 WIRE',
      'SET 0 0 0 TORCH SOUTH',
      'SET 0 0 -1 WIRE',
      'SET 0 0 -2 SWITCH UP 1',
      'TICK 2'
    ).should =~ /\(0,0,1\) POWER 0\n/
  end

  it 'passes power up through a conductor' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 1 0 CONDUCTOR',
      'SET 0 2 0 WIRE',
      'TICK 2'
    )
    result.should =~ /\(0,1,0\) POWER 15/
    result.should =~ /\(0,2,0\) POWER 15/
  end
end
