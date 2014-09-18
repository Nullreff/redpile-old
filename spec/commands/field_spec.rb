require 'spec_helper'
include Helpers

FIELD_RANGE1 = (-1..1).map do |x|
  (-1..1).map do |y|
    (-1..1).map do |z|
      "(#{x},#{y},#{z}) 10"
    end
  end
end.flatten.flatten.join("\n")

FIELD_RANGE2 = (-1..1).map do |x|
  (-1..1).map do |y|
    (-1..1).map do |z|
      "(#{x * 2},#{y * 2},#{z * 2}) 10"
    end
  end
end.flatten.flatten.join("\n")

describe 'NODE' do
  context 'sets' do
    it 'a field on a single node' do
      run(
        'NODE 0,0,0 WIRE',
        'FIELD 0,0,0 power:10'
      ).should == ''
    end

    it 'a field on a range of nodes' do
      run(
        'NODE -5..5,-5..5,-5..5 WIRE',
        'FIELD -5..5,-5..5,-5..5 power:10'
      ).should == ''
    end

    it 'a field on a range of nodes with step' do
      run(
        'NODE -10..10%2,-10..10%2,-10..10%2 WIRE',
        'FIELD -10..10%2,-10..10%2,-10..10%2 power:10'
      ).should == ''
    end
  end

  context 'gets' do
    it 'a field on a single node' do
      run(
        'NODE 0,0,0 WIRE power:10',
        'FIELD 0,0,0 power'
      ).should == '(0,0,0) 10'
    end

    it 'a field on a range of nodes' do
      run(
        'NODE -1..1,-1..1,-1..1 WIRE power:10',
        'FIELD -1..1,-1..1,-1..1 power'
      ).should == FIELD_RANGE1
    end

    it 'a field on a range of nodes with step' do
      run(
        'NODE -2..2%2,-2..2%2,-2..2%2 WIRE power:10',
        'FIELD -2..2%2,-2..2%2,-2..2%2 power'
      ).should == FIELD_RANGE2
    end
  end

  it 'errors if given an incorrect direction' do
    run(
      'NODE 0,0,0 TORCH',
      'FIELD 0,0,0 direction:INVALID'
    ).should == "'INVALID' is not a direction"
  end
end
