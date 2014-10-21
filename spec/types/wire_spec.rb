require 'spec_helper'
include Helpers

describe 'Wire' do
  it 'propigates power to the side and down' do
    run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 AIR',
      'NODE 0,-1,2 WIRE',
      'TICK'
    ).should =~ /0,-1,2 FIELD power:14/
  end

  it 'propigates power to the side and up' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 CONDUCTOR',
      'NODE 0,1,2 WIRE',
      'TICK'
    ).should =~ /0,1,2 FIELD power:14/
  end

  it 'does not propigate to the side and up when a block is on top' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 CONDUCTOR',
      'NODE 0,1,2 WIRE',
      'NODE 0,1,1 CONDUCTOR',
      'TICK',
      'NODE 0,1,2'
    )
    contains_node?(result, 0, 1, 2, 'WIRE', power: 0)
  end

  it 'powers an adjacent conductor' do
    run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 CONDUCTOR',
      'TICK'
    ).should =~ /0,0,2 FIELD power:14/
  end

  it 'diverts power from a condutor when a wire is on the left' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 1,0,1 WIRE',
      'NODE 0,0,2 CONDUCTOR',
      'TICK',
      'NODE 0,0,2'
    )
    contains_node?(result, 0, 0, 2, 'CONDUCTOR', power: 0)
  end

  it 'diverts power from a condutor when a wire is on the right' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE -1,0,1 WIRE',
      'NODE 0,0,2 CONDUCTOR',
      'TICK',
      'NODE 0,0,2'
    )
    contains_node?(result, 0, 0, 2, 'CONDUCTOR', power: 0)
  end

  it 'diverts power from a condutor when a wire is on both sides' do
    result = run(
       'NODE 0,0,0 SWITCH direction:UP state:1',
       'NODE 0,0,1 WIRE',
       'NODE 1,0,1 WIRE',
       'NODE -1,0,1 WIRE',
       'NODE 0,0,2 CONDUCTOR',
       'TICK',
      'NODE 0,0,2'
    )
    contains_node?(result, 0, 0, 2, 'CONDUCTOR', power: 0)
  end

  it 'does not divert power from a conductor if there is a wire on top' do
    result = run(
       'NODE 0,0,0 SWITCH direction:UP state:1',
       'NODE 0,0,1 WIRE',
       'NODE 1,0,1 WIRE',
       'NODE -1,0,1 WIRE',
       'NODE 0,0,2 CONDUCTOR',
       'NODE 0,1,2 WIRE',
       'TICK',
       'NODE 0,0,2'
    )
    contains_node?(result, 0, 0, 2, 'CONDUCTOR', power: 14)
  end

  it 'propigates power to a conductor underneath wire' do
    run(
       'NODE 0,0,0 SWITCH direction:UP state:1',
       'NODE 0,0,1 WIRE',
       'NODE 0,-1,1 CONDUCTOR',
       'TICK'
    ).should =~ /0,-1,1 FIELD power:15/
  end

  it 'propigates power in a loop' do
    result = run(
      'NODE 0,0,0 SWITCH direction:UP state:1',
      'NODE 0,0,1 WIRE',
      'NODE 0,0,2 WIRE',
      'NODE 1,0,2 WIRE',
      'NODE 2,0,2 WIRE',
      'NODE 2,0,1 WIRE',
      'NODE 2,0,0 WIRE',
      'NODE 1,0,0 WIRE',
      'TICK'
    )
    result.should =~ /0,0,1 FIELD power:15$/
    result.should =~ /0,0,2 FIELD power:14$/
    result.should =~ /1,0,2 FIELD power:13$/
    result.should =~ /2,0,2 FIELD power:12$/
    result.should =~ /2,0,1 FIELD power:13$/
    result.should =~ /2,0,0 FIELD power:14$/
    result.should =~ /1,0,0 FIELD power:15$/
  end

  it 'avoids looping when processing a grid of wire' do
    run(
      'NODER -5..5,0,-5..5 WIRE',
      'NODE 0,0,0 TORCH direction:UP',
      'TICK 2'
    ).should_not =~ /^Logic loop detected while performing tick$/
  end
end
