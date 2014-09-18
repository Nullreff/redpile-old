require 'spec_helper'
include Helpers

describe 'MESSAGES' do
  it 'displays no messages when empty' do
    run('MESSAGES').should == ''
  end

  it 'prints a list of messages after a tick' do
    run(
      'NODE 0,0,0 TORCH direction:UP',
      'NODE 0,0,1..5 WIRE',
      'TICK',
      'MESSAGES'
    ).should =~ /^\d \(0,0,0\) => \(\d,\d,\d\) POWER \d+$/
  end
end
