require 'spec_helper'
include Helpers

describe 'TYPES' do
  it 'displays a list of types' do
    run('TYPES').should =~ /([a-zA-Z0-9]+\n(  \d+: [a-zA-Z0-9]+ (INTEGER|DIRECTION|STRING)\n)+)+/m
  end
end

