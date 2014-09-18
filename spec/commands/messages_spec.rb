require 'spec_helper'
include Helpers

describe 'MESSAGES' do
  it 'displays no messages when empty' do
    run('MESSAGES').should == "\n"
  end

  it 'prints a list of messages after a tick' do
    run(
      'NODE 0 0 0 TORCH direction:UP',
      'NODER 0 0 1 0 0 5 WIRE',
      'TICK',
      'MESSAGES'
    ).should =~ /^\d \(0,0,0\) => \(\d,\d,\d\) POWER \d+$/
  end
end
