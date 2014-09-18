require 'spec_helper'
include Helpers

describe 'TICK' do
  it 'runs a standard tick' do
    run('TICK').should == "\n"
  end

  it 'runs a verbose tick' do
    run('TICKV').should =~ /^--- Tick 0 ---$/
  end

  it 'runs a quiet tick' do
    run('TICKQ').should == "\n"
  end

  it 'runs multiple ticks' do
    run('TICK 4', 'STATUS').should =~ /^ticks: 4$/
  end

  it 'errors for negative ticks' do
    run('TICK -2').should =~ /^Tick count must be greater than zero$/
  end

  it 'errors for non numerical ticks' do
    run('TICK abc').should =~ /^Tick count must be numeric$/
  end
end
