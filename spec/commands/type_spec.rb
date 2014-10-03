require 'spec_helper'
include Helpers

describe 'TYPE' do
  it 'displays a list of types' do
    run('TYPE').should == 
%q{ECHO
SWITCH
COMPARATOR
REPEATER
PISTON
TORCH
CONDUCTOR
WIRE
INSULATOR
AIR}
  end

  it 'displays the information for a type' do
    result = run('TYPE TORCH')
    result.should =~ /^Name: TORCH$/
    result.should =~ /Fields:\n(  \d+: [a-zA-Z0-9_]+ (INTEGER|DIRECTION|STRING)\n)+/m
    result.should =~ /Behaviors:\n(  \d+: [a-zA-Z0-9_]+\n)+/m
  end
end

