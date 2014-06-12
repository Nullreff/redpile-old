require 'spec_helper'
include Helpers

describe 'Piston' do
  it 'extends a conductor when powered' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 PISTON SOUTH',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 0 3 AIR',
      'TICK 3',
      'GET 0 0 2',
      'GET 0 0 3'
    ).should =~ /AIR NORTH 0 0\n.*CONDUCTOR NORTH 0 0\n/m
  end

  it 'extends a conductor into a empty block' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 PISTON SOUTH',
      'SET 0 0 2 CONDUCTOR',
      'TICK 3',
      'GET 0 0 2',
      'GET 0 0 3'
    ).should =~ /AIR NORTH 0 0\n.*CONDUCTOR NORTH 0 0\n/m
  end

  it 'extends and retracts a conductor' do
    run(
      'SET 0 0 0 TORCH UP',
      'SET 0 0 1 PISTON SOUTH',
      'SET 0 0 2 CONDUCTOR',
      'SET 0 0 3 AIR',
      'TICK 3',
      'SET 0 0 0 EMPTY',
      'TICK 3',
      'GET 0 0 2',
      'GET 0 0 3'
    ).should =~ /CONDUCTOR NORTH 0 0\n.*AIR NORTH 0 0\n/m
  end
end

