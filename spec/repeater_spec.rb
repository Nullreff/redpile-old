require 'spec_helper'
include Helpers

describe 'Repeater' do
  ['wire', 'conductor'].each do |material|
    it "passes power to a #{material}" do
      run(
        'SET 0 0 0 TORCH UP',
        'SET 0 0 1 REPEATER SOUTH 0',
        "SET 0 0 2 #{material}",
        'TICK 3'
      ).should =~ /\(0,0,2\) 15/
    end
  end

  (1..4).each do |delay|
    it "delays the propigation of power by #{delay} tick(s)" do
      run(
        'SET 0 0 0 TORCH UP',
        "SET 0 0 1 REPEATER SOUTH #{delay - 1}",
        'SET 0 0 2 WIRE',
        "TICK #{delay + 1}",
        'GET 0 0 2',
        'TICK',
        'GET 0 0 2'
      ).should =~ /WIRE NORTH 0 0\n.*WIRE NORTH 15 0\n/m
    end
  end

  it 'is blocked from being powered by a repeater on the left' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET -1 0 2 TORCH UP',
      'SET 0 0 1 REPEATER SOUTH 0',
      'SET 0 0 2 REPEATER EAST 0',
      'SET 1 0 2 WIRE',
      'TICK 4',
      'GET 1 0 2'
    ).should =~ /WIRE NORTH 0 0\n/
  end

  it 'is blocked from being powered by a repeater on the right' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 1 0 2 TORCH UP',
      'SET 0 0 1 REPEATER SOUTH 0',
      'SET 0 0 2 REPEATER WEST 0',
      'SET -1 0 2 WIRE',
      'TICK 4',
      'GET -1 0 2'
    ).should =~ /WIRE NORTH 0 0\n/
  end
end
