require 'spec_helper'
include Helpers

describe 'Piston' do
  %w(CONDUCTOR INSULATOR).each do |type|
    context type do
      it 'is pushed one block' do
        result = run(
          'SET 0 0 0 SWITCH UP 1',
          'SET 0 0 1 PISTON SOUTH',
          "SET 0 0 2 #{type}",
          'TICKQ 2',
          'GET 0 0 2',
          'GET 0 0 3'
        )
        result.should =~ /^\(0,0,2\) AIR$/
        result.should =~ /^\(0,0,3\) #{type}/
      end

      it 'is pulled one block' do
        result = run(
          'SET 0 0 0 SWITCH UP 1',
          'SET 0 0 1 PISTON SOUTH',
          "SET 0 0 2 #{type}",
          'TICKQ 2',
          'SET 0 0 0 SWITCH UP 0',
          'TICKQ 2',
          'GET 0 0 2',
          'GET 0 0 3'
        )
        result.should =~ /^\(0,0,2\) #{type}/
        result.should =~ /^\(0,0,3\) AIR$/
      end
    end
  end

  %w(WIRE TORCH REPEATER COMPARATOR SWITCH).each do |type|
    context type do
      it 'breaks when pushed' do
        result = run(
          'SET 0 0 0 SWITCH UP 1',
          'SET 0 0 1 PISTON SOUTH',
          "SET 0 0 2 #{type}",
          'SET 0 0 3 AIR',
          'TICKQ 2',
          'GET 0 0 2',
          'GET 0 0 3'
        )
        result.should =~ /^\(0,0,2\) EMPTY$/
        result.should =~ /^\(0,0,3\) AIR$/
      end

      it 'does nothing when pulled' do
        result = run(
          'SET 0 0 0 SWITCH UP 1',
          'SET 0 0 1 PISTON SOUTH',
          'SET 0 0 2 AIR',
          "SET 0 0 3 #{type}",
          'TICKQ 2',
          'SET 0 0 0 SWITCH UP 0',
          'TICKQ 2',
          'GET 0 0 2',
          'GET 0 0 3'
        )
        result.should =~ /^\(0,0,2\) AIR$/
        result.should =~ /^\(0,0,3\) #{type}/
      end
    end
  end
end

