require 'spec_helper'
include Helpers

describe 'STATUS' do
  it 'displays the number of ticks' do
    run('STATUS').should =~ /^ticks: 0$/
  end

  it 'displays the number of blocks' do
    run(
      'NODE 0 0 0 AIR',
      'NODE 0 0 1 WIRE',
      'STATUS'
    ).should =~ /^nodes: 2$/
  end

  it 'doesn\'t count overlapping blocks' do
    run(
      'NODE 0 0 0 AIR',
      'NODE 0 0 0 WIRE',
      'STATUS'
    ).should =~ /^nodes: 1$/
  end

  it 'detects removed blocks' do
    run(
      'NODE 0 0 0 WIRE',
      'NODE 0 0 1 WIRE',
      'DELETE 0 0 0',
      'STATUS'
    ).should =~ /^nodes: 1$/
  end
end
