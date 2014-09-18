require 'spec_helper'
include Helpers

describe 'Echo' do
  it 'Stores a message' do
    result = run(
      'NODE 0,0,0 ECHO message:Hello',
      'NODE 0,0,0'
    )
    contains_node?(result, 0, 0, 0, 'ECHO', message: '"Hello"')
  end

  it 'Stores a message with quotes' do
    result = run(
      'NODE 0,0,0 ECHO message:"Hello world"',
      'NODE 0,0,0'
    )
    contains_node?(result, 0, 0, 0, 'ECHO', message: '"Hello world"')
  end

  it 'Stores a message with escaped quotes' do
    result = run(
      'NODE 0,0,0 ECHO message:"Hello \"Quoted\" world"',
      'NODE 0,0,0'
    )
    contains_node?(result, 0, 0, 0, 'ECHO', message: '"Hello \\\"Quoted\\\" world"')
  end

  it 'Stores and empty message' do
    result = run(
      'NODE 0,0,0 ECHO',
      'NODE 0,0,0'
    )
    contains_node?(result, 0, 0, 0, 'ECHO', message: '""')
  end

  it 'Prints a message when powered' do
    run(
      'NODE 0,0,0 SWITCH direction:up state:1',
      'NODE 0,0,1 ECHO message:Hello',
      'TICK'
    ).should =~ /\(0,0,1\) DATA "Hello"/
  end

  it 'Does not print a message when unpowered' do
    run(
      'NODE 0,0,0 SWITCH direction:up state:0',
      'NODE 0,0,1 ECHO message:Hello',
      'TICK'
    ).should_not =~ /\(0,0,1\) DATA "Hello"/
  end

  it 'Does not print an empty message when powered' do
    run(
      'NODE 0,0,0 SWITCH direction:up state:1',
      'NODE 0,0,1 ECHO',
      'TICK'
    ).should_not =~ /\(0,0,1\) DATA ""/
  end
end

