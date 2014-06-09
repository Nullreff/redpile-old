require 'spec_helper'
include Helpers

describe 'Wire' do
  it 'propigates power to the side and down' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 AIR',
      'SET 0 -1 2 WIRE',
      'TICK 2'
    ).should =~ /\(0,-1,2\) 14/
  end

  it 'propigates power to the side and up' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 1 2 WIRE',
      'TICK 2'
    ).should =~ /\(0,1,2\) 14/
  end

  it 'does not propigate to the side and up when a block is on top' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 1 2 WIRE',
      'SET 0 1 1 CONDUCTOR',
      'TICK 2'
    ).should_not =~ /\(0,1,2\)/
  end

  it 'powers an adjacent conductor' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'TICK 2'
    ).should =~ /\(0,0,2\) 15/
  end

  it 'diverts power from a condutor when a wire is on the left' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET 1 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'TICK 2',
      'GET 0 0 2'
    )
    result.should =~ /\(0,0,2\) 0 CONDUCTOR/
  end

  it 'diverts power from a condutor when a wire is on the right' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 WIRE',
      'SET -1 0 1 WIRE',
      'SET 0 0 2 CONDUCTOR',
      'TICK 2',
      'GET 0 0 2'
    )
    result.should =~ /\(0,0,2\) 0 CONDUCTOR/
  end

  it 'diverts power from a condutor when a wire is on both sides' do
    result = run(
       'SET 0 0 0 TORCH UP',
       'SET 0 0 1 WIRE',
       'SET 1 0 1 WIRE',
       'SET -1 0 1 WIRE',
       'SET 0 0 2 CONDUCTOR',
       'TICK 2',
      'GET 0 0 2'
    )
    result.should =~ /\(0,0,2\) 0 CONDUCTOR/
  end

  it 'does not divert power from a conductor if there is a wire on top' do
    result = run(
       'SET 0 0 0 TORCH UP',
       'SET 0 0 1 WIRE',
       'SET 1 0 1 WIRE',
       'SET -1 0 1 WIRE',
       'SET 0 0 2 CONDUCTOR',
       'SET 0 1 2 WIRE',
       'TICK 2',
       'GET 0 0 2'
    )
    result.should =~ /\(0,0,2\) 14 CONDUCTOR/
  end

  it 'propigates power to a conductor underneath wire' do
    run(
       'SET 0 0 0 TORCH UP',
       'SET 0 0 1 WIRE',
       'SET 0 -1 1 CONDUCTOR',
       'TICK 2'
    ).should =~ /\(0,-1,1\) 15/
  end
end
