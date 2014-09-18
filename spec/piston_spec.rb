require 'spec_helper'
include Helpers

describe 'Piston' do
  %w(CONDUCTOR INSULATOR PISTON).each do |type|
    context "moving #{type}" do
      it 'is pushed one block' do
        result = run(
          'NODE 0 0 0 SWITCH direction:UP state:1',
          'NODE 0 0 1 PISTON direction:SOUTH',
          "NODE 0 0 2 #{type}",
          'TICKQ 2',
          'NODE 0 0 2',
          'NODE 0 0 3'
        )
        contains_node?(result, 0, 0, 2, 'AIR')
        contains_node?(result, 0, 0, 3, type)
      end

      it 'is pulled one block' do
        result = run(
          'NODE 0 0 0 SWITCH direction:UP state:1',
          'NODE 0 0 1 PISTON direction:SOUTH',
          "NODE 0 0 3 #{type}",
          'TICKQ 2',
          'NODE 0 0 0 SWITCH direction:UP state:0',
          'TICKQ 2',
          'NODE 0 0 2',
          'NODE 0 0 3'
        )
        contains_node?(result, 0, 0, 2, type)
        contains_node?(result, 0, 0, 3, 'AIR')
      end
    end
  end

  %w(WIRE TORCH REPEATER COMPARATOR SWITCH).each do |type|
    context "moving #{type}" do
      it 'breaks when pushed' do
        result = run(
          'NODE 0 0 0 SWITCH direction:UP state:1',
          'NODE 0 0 1 PISTON direction:SOUTH',
          "NODE 0 0 2 #{type}",
          'NODE 0 0 3 AIR',
          'TICKQ 2',
          'NODE 0 0 2',
          'NODE 0 0 3'
        )
        contains_node?(result, 0, 0, 2, 'AIR')
        contains_node?(result, 0, 0, 3, 'AIR')
      end

      it 'does nothing when pulled' do
        result = run(
          'NODE 0 0 0 SWITCH direction:UP state:1',
          'NODE 0 0 1 PISTON direction:SOUTH',
          'NODE 0 0 2 AIR',
          "NODE 0 0 3 #{type}",
          'TICKQ 2',
          'NODE 0 0 0 SWITCH direction:UP state:0',
          'TICKQ 2',
          'NODE 0 0 2',
          'NODE 0 0 3'
        )
        contains_node?(result, 0, 0, 2, 'AIR')
        contains_node?(result, 0, 0, 3, type)
      end
    end
  end

  context 'moving extended PISTON' do
    it 'does not move when pushed' do
      result = run(
        'NODE 0 0 1 PISTON direction:SOUTH',
        'NODE 0 0 2 PISTON direction:EAST',
        'NODE 1 0 2 CONDUCTOR',
        'NODE -1 0 2 SWITCH direction:UP state:1',
        'TICKQ 2',
        'NODE 0 0 0 SWITCH direction:UP state:1',
        'TICKQ 2',
        'NODE 0 0 2',
        'NODE 0 0 3'
      )
      contains_node?(result, 0, 0, 2, 'PISTON', power: 15, direction: 'EAST')
      contains_node?(result, 0, 0, 3, 'AIR')
    end

    it 'does not move when pulled' do
      result = run(
        'NODE 0 0 1 PISTON direction:SOUTH',
        'NODE 0 0 3 PISTON direction:EAST',
        'NODE 1 0 3 CONDUCTOR',
        'NODE -1 0 3 SWITCH direction:UP state:1',
        'TICKQ 2',
        'NODE 0 0 0 SWITCH direction:UP state:1',
        'TICKQ 2',
        'NODE 0 0 0 SWITCH direction:UP state:0',
        'TICKQ 2',
        'NODE 0 0 2',
        'NODE 0 0 3'
      )
      contains_node?(result, 0, 0, 2, 'AIR')
      contains_node?(result, 0, 0, 3, 'PISTON', power: 15, direction: 'EAST')
    end
  end
end

