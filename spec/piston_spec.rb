require 'spec_helper'
include Helpers

describe 'Piston' do
  it 'extends a conductor when powered' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 PISTON SOUTH',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 0 3 AIR',
      'TICK 3',
      'GET 0 0 2',
      'GET 0 0 3'
    )
    result.should =~ /^\(0,0,2\) AIR$/
    result.should =~ /^\(0,0,3\) CONDUCTOR 0$/
  end

  it 'extends a conductor into a empty block' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 PISTON SOUTH',
      'SET 0 0 2 CONDUCTOR',
      'TICK 3',
      'GET 0 0 2',
      'GET 0 0 3'
    )
    result.should =~ /^\(0,0,2\) AIR$/
    result.should =~ /^\(0,0,3\) CONDUCTOR 0$/
  end

  it 'extends and retracts a conductor' do
    result = run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 PISTON SOUTH',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 0 3 AIR',
      'TICK 3',
      'SET 0 0 0 EMPTY',
      'TICK 3',
      'GET 0 0 2',
      'GET 0 0 3'
    )
    result.should =~ /^\(0,0,2\) CONDUCTOR 0$/
    result.should =~ /^\(0,0,3\) AIR$/
  end
end

