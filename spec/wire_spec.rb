require 'spec_helper'
include Helpers

describe 'Wire' do
  it 'propigates power to the side and down' do
    run(
      'SET 0 0 0 SWITCH UP 1',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 AIR',
      'SET 0 -1 2 WIRE',
      'TICK'
    ).should =~ /\(0,-1,2\) POWER 14/
  end

  it 'propigates power to the side and up' do
    run(
      'SET 0 0 0 SWITCH UP 1',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 1 2 WIRE',
      'TICK'
    ).should =~ /\(0,1,2\) POWER 14/
  end

  it 'does not propigate to the side and up when a block is on top' do
    run(
      'SET 0 0 0 SWITCH UP 1',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 1 2 WIRE',
      'SET 0 1 1 CONDUCTOR',
      'TICK',
      'GET 0 1 2'
    ).should =~ /^\(0,1,2\) WIRE 0$/
  end

  it 'powers an adjacent conductor' do
    run(
      'SET 0 0 0 SWITCH UP 1',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'TICK'
    ).should =~ /\(0,0,2\) POWER 14/
  end

  it 'diverts power from a condutor when a wire is on the left' do
    result = run(
      'SET 0 0 0 SWITCH UP 1',
      'SET 0 0 1 WIRE',
      'SET 1 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'TICK',
      'GET 0 0 2'
    )
    result.should =~ /^\(0,0,2\) CONDUCTOR 0$/
  end

  it 'diverts power from a condutor when a wire is on the right' do
    result = run(
      'SET 0 0 0 SWITCH UP 1',
      'SET 0 0 1 WIRE',
      'SET -1 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'TICK',
      'GET 0 0 2'
    )
    result.should =~ /^\(0,0,2\) CONDUCTOR 0$/
  end

  it 'diverts power from a condutor when a wire is on both sides' do
    result = run(
       'SET 0 0 0 SWITCH UP 1',
       'SET 0 0 1 WIRE',
       'SET 1 0 1 WIRE',
       'SET -1 0 1 WIRE',
       'SET 0 0 2 CONDUCTOR',
       'TICK',
      'GET 0 0 2'
    )
    result.should =~ /^\(0,0,2\) CONDUCTOR 0$/
  end

  it 'does not divert power from a conductor if there is a wire on top' do
    result = run(
       'SET 0 0 0 SWITCH UP 1',
       'SET 0 0 1 WIRE',
       'SET 1 0 1 WIRE',
       'SET -1 0 1 WIRE',
       'SET 0 0 2 CONDUCTOR',
       'SET 0 1 2 WIRE',
       'TICK',
       'GET 0 0 2'
    )
    result.should =~ /^\(0,0,2\) CONDUCTOR 14$/
  end

  it 'propigates power to a conductor underneath wire' do
    run(
       'SET 0 0 0 SWITCH UP 1',
       'SET 0 0 1 WIRE',
       'SET 0 -1 1 CONDUCTOR',
       'TICK'
    ).should =~ /\(0,-1,1\) POWER 15/
  end

  it 'propigates power in a loop' do
    result = run(
      'set 0 0 0 SWITCH UP 1',
      'set 0 0 1 wire',
      'set 0 0 2 wire',
      'set 1 0 2 wire',
      'set 2 0 2 wire',
      'set 2 0 1 wire',
      'set 2 0 0 wire',
      'set 1 0 0 wire',
      'tick'
    )
    result.should =~ /^\(0,0,1\) POWER 15$/
    result.should =~ /^\(0,0,2\) POWER 14$/
    result.should =~ /^\(1,0,2\) POWER 13$/
    result.should =~ /^\(2,0,2\) POWER 12$/
    result.should =~ /^\(2,0,1\) POWER 13$/
    result.should =~ /^\(2,0,0\) POWER 14$/
    result.should =~ /^\(1,0,0\) POWER 15$/
  end
end
