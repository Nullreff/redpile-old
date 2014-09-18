require 'spec_helper'
include Helpers

describe 'NODE' do
  context 'sets' do
    it 'a single node' do
      run('NODE 0,0,0 TORCH direction:UP').should == "\n"
    end

    it 'a range of nodes' do
      run('NODE -5..5,-5..5,-5..5 TORCH direction:UP').should == "\n"
    end

    it 'a range of nodes with step' do
      run('NODE -10..10%2,-10..10%2,-10..10%2 TORCH direction:UP').should == "\n"
    end
  end

  context 'gets' do
    it 'a signle node' do
      run('NODE 0,0,0').should =~ /^\(0,0,0\) AIR$/
    end
  end

  it 'errors if given an incorrect direction' do
    run('NODE 0,0,0 TORCH direction:INVALID').should =~ /^'INVALID' is not a direction$/
  end

  it 'errors if given an incorrect type' do
    run('NODE 0,0,0 INVALID').should =~ /^Unknown type 'INVALID'$/
  end

  context 'inserts' do
    %w(AIR INSULATOR).each do |type|
      it type do
        result = run(
          "NODE 0,0,0 #{type}",
          "NODE 0,0,0"
        )
        contains_node?(result, 0, 0, 0, type)
      end
    end

    %w(WIRE CONDUCTOR ECHO).each do |type|
      it type do
        result = run(
          "NODE 0,0,0 #{type}",
          "NODE 0,0,0"
        )
        contains_node?(result, 0, 0, 0, type, power: 0)
      end
    end

    %w(TORCH PISTON).each do |type|
      DIRECTIONS.each do |dir|
        it "#{type} pointing #{dir}" do
          result = run(
            "NODE 0,0,0 #{type} direction:#{dir}",
            "NODE 0,0,0"
          )
          contains_node?(result, 0, 0, 0, type, power: 0, direction: dir)
        end
      end
    end

    %w(REPEATER COMPARATOR SWITCH).each do |type|
      DIRECTIONS.each do |dir|
        it "#{type} pointing #{dir}" do
          result = run(
            "NODE 0,0,0 #{type} direction:#{dir} state:1",
            "NODE 0,0,0"
          )
          contains_node?(result, 0, 0, 0, type, power: 0, direction: dir, state: 1)
        end
      end
    end
  end
end
