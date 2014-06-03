require 'spec_helper'
include Helpers

describe 'Torch' do
  MAX_RANGE = 15
  (1..MAX_RANGE).each do |range|
    it "propigates power #{range} blocks" do
      run(
        'SET 0 0 0 TORCH UP',
        *(1..range).map {|r| "SET 0 0 #{r} WIRE"},
        'TICK'
      ).should =~ /\(0,0,#{range}\) #{16 - range}\n/
    end
  end

  it "stops propigating power after #{MAX_RANGE} blocks" do
    end_block = MAX_RANGE + 1
    run(
      'SET 0 0 0 TORCH UP',
      *(1..end_block).map {|r| "SET 0 0 #{r} WIRE"},
      'TICK',
      "GET 0 0 #{end_block}"
    ).should =~ /\(0,0,#{end_block}\) 0 WIRE\n/
  end

  it 'takes power from the closer torch' do
    run(
      'SET 0 0 -3 TORCH NORTH',
      'SET 0 0 -2 WIRE',
      'SET 0 0 -1 WIRE',
      'SET 0 0 0 WIRE',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 TORCH NORTH',
      'TICK'
    ).should =~ /\(0,0,0\) 14\n/
  end

  it 'turns a torch off with power' do
    run(
      'SET 0 0 1 WIRE',
      'SET 0 0 0 TORCH SOUTH',
      'SET 0 0 -1 WIRE',
      'SET 0 0 -2 TORCH UP',
      'TICK 2'
    ).should =~ /\(0,0,1\) 0\n/
  end

  it 'passes power up through a conductor' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 1 0 CONDUCTOR',
      'SET 0 2 0 WIRE',
      'TICK'
    )
    result.should =~ /\(0,1,0\) 15/
    result.should =~ /\(0,2,0\) 15/
  end
end
