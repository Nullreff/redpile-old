require 'spec_helper'
include Helpers

describe 'Echo' do
  it 'Stores a message' do
    result = run(
      'SET 0 0 0 ECHO message:Hello',
      'GET 0 0 0'
    )
    contains_node?(result, 0, 0, 0, 'ECHO', message: '"Hello"')
  end

  it 'Stores a message with quotes' do
    result = run(
      'SET 0 0 0 ECHO message:"Hello world"',
      'GET 0 0 0'
    )
    contains_node?(result, 0, 0, 0, 'ECHO', message: '"Hello world"')
  end

  it 'Prints a message when powered' do
    run(
      'SET 0 0 0 SWITCH direction:up state:1',
      'SET 0 0 1 ECHO message:Hello',
      'TICK'
    ).should =~ /\(0,0,1\) DATA "Hello"/
  end

  it 'Does not print a message when unpowered' do
    run(
      'SET 0 0 0 SWITCH direction:up state:0',
      'SET 0 0 1 ECHO message:Hello',
      'TICK'
    ).should_not =~ /\(0,0,1\) DATA "Hello"/
  end
end

