require 'spec_helper'
include Helpers

describe 'Piston' do
  %w(CONDUCTOR INSULATOR PISTON).each do |type|
    context "moving #{type}" do
      it 'is pushed one block' do
        result = run(
          'SET 0 0 0 SWITCH direction:UP state:1',
          'SET 0 0 1 PISTON direction:SOUTH',
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
          'SET 0 0 0 SWITCH direction:UP state:1',
          'SET 0 0 1 PISTON direction:SOUTH',
          "SET 0 0 3 #{type}",
          'TICKQ 2',
          'SET 0 0 0 SWITCH direction:UP state:0',
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
    context "moving #{type}" do
      it 'breaks when pushed' do
        result = run(
          'SET 0 0 0 SWITCH direction:UP state:1',
          'SET 0 0 1 PISTON direction:SOUTH',
          "SET 0 0 2 #{type}",
          'SET 0 0 3 AIR',
          'TICKQ 2',
          'GET 0 0 2',
          'GET 0 0 3'
        )
        result.should =~ /^\(0,0,2\) AIR$/
        result.should =~ /^\(0,0,3\) AIR$/
      end

      it 'does nothing when pulled' do
        result = run(
          'SET 0 0 0 SWITCH direction:UP state:1',
          'SET 0 0 1 PISTON direction:SOUTH',
          'SET 0 0 2 AIR',
          "SET 0 0 3 #{type}",
          'TICKQ 2',
          'SET 0 0 0 SWITCH direction:UP state:0',
          'TICKQ 2',
          'GET 0 0 2',
          'GET 0 0 3'
        )
        result.should =~ /^\(0,0,2\) AIR$/
        result.should =~ /^\(0,0,3\) #{type}/
      end
    end
  end

  context 'moving extended PISTON' do
    it 'does not move when pushed' do
      result = run(
        'SET 0 0 1 PISTON direction:SOUTH',
        'SET 0 0 2 PISTON direction:EAST',
        'SET 1 0 2 CONDUCTOR',
        'SET -1 0 2 SWITCH direction:UP state:1',
        'TICKQ 2',
        'SET 0 0 0 SWITCH direction:UP state:1',
        'TICKQ 2',
        'GET 0 0 2',
        'GET 0 0 3'
      )
      result.should =~ /^\(0,0,2\) PISTON 15 EAST$/
      result.should =~ /^\(0,0,3\) AIR$/
    end
    it 'does not move when pulled' do
      result = run(
        'SET 0 0 1 PISTON direction:SOUTH',
        'SET 0 0 3 PISTON direction:EAST',
        'SET 1 0 3 CONDUCTOR',
        'SET -1 0 3 SWITCH direction:UP state:1',
        'TICKQ 2',
        'SET 0 0 0 SWITCH direction:UP state:1',
        'TICKQ 2',
        'SET 0 0 0 SWITCH direction:UP state:0',
        'TICKQ 2',
        'GET 0 0 2',
        'GET 0 0 3'
      )
      result.should =~ /^\(0,0,2\) AIR$/
      result.should =~ /^\(0,0,3\) PISTON 15 EAST$/
    end
  end
end

