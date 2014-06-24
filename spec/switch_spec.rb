require 'spec_helper'
include Helpers

describe 'Switch' do
  context 'turned on' do
    it 'powers a conductor behind it' do
      run(
        'SET 0 0 0 SWITCH UP 1',
        'SET 0 -1 0 CONDUCTOR',
        'TICK',
        'GET 0 -1 0'
      ).should =~ /^\(0,-1,0\) CONDUCTOR 15$/
    end

    it 'does not power a conductor to the side' do
      run(
        'SET 0 0 0 SWITCH UP 1',
        'SET 1 0 0 CONDUCTOR',
        'TICK',
        'GET 1 0 0'
      ).should =~ /^\(1,0,0\) CONDUCTOR 0$/
    end

    it 'powers a wire to the side of it' do
      run(
        'SET 0 0 0 SWITCH UP 1',
        'SET 1 0 0 WIRE',
        'TICK',
        'GET 1 0 0'
      ).should =~ /^\(1,0,0\) WIRE 15$/
    end
  end

  context 'turned off' do
    it 'does not power a conductor behind it' do
      run(
        'SET 0 0 0 SWITCH UP 0',
        'SET 0 -1 0 CONDUCTOR',
        'TICK',
        'GET 0 -1 0'
      ).should =~ /^\(0,-1,0\) CONDUCTOR 0$/
    end

    it 'does not power a conductor to the side' do
      run(
        'SET 0 0 0 SWITCH UP 0',
        'SET 1 0 0 CONDUCTOR',
        'TICK',
        'GET 1 0 0'
      ).should =~ /^\(1,0,0\) CONDUCTOR 0$/
    end

    it 'does not power a wire to the side of it' do
      run(
        'SET 0 0 0 SWITCH UP 0',
        'SET 1 0 0 WIRE',
        'TICK',
        'GET 1 0 0'
      ).should =~ /^\(1,0,0\) WIRE 0$/
    end
  end
end
