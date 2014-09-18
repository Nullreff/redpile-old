require 'spec_helper'
include Helpers

describe 'Switch' do
  context 'turned on' do
    it 'powers a conductor behind it' do
      result = run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        'NODE 0,-1,0 CONDUCTOR',
        'TICK',
        'NODE 0,-1,0'
      )
      contains_node?(result, 0, -1, 0, 'CONDUCTOR', power: 15)
    end

    it 'does not power a conductor to the side' do
      result = run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        'NODE 1,0,0 CONDUCTOR',
        'TICK',
        'NODE 1,0,0'
      )
      contains_node?(result, 1, 0, 0, 'CONDUCTOR', power: 0)
    end

    it 'powers a wire to the side of it' do
      result = run(
        'NODE 0,0,0 SWITCH direction:UP state:1',
        'NODE 1,0,0 WIRE',
        'TICK',
        'NODE 1,0,0'
      )
      contains_node?(result, 1, 0, 0, 'WIRE', power: 15)
    end
  end

  context 'turned off' do
    it 'does not power a conductor behind it' do
      result = run(
        'NODE 0,0,0 SWITCH direction:UP state:0',
        'NODE 0,-1,0 CONDUCTOR',
        'TICK',
        'NODE 0,-1,0'
      )
      contains_node?(result, 0, -1, 0, 'CONDUCTOR', power: 0)
    end

    it 'does not power a conductor to the side' do
      result = run(
        'NODE 0,0,0 SWITCH direction:UP state:0',
        'NODE 1,0,0 CONDUCTOR',
        'TICK',
        'NODE 1,0,0'
      )
      contains_node?(result, 1, 0, 0, 'CONDUCTOR', power: 0)
    end

    it 'does not power a wire to the side of it' do
      result = run(
        'NODE 0,0,0 SWITCH direction:UP state:0',
        'NODE 1,0,0 WIRE',
        'TICK',
        'NODE 1,0,0'
      )
      contains_node?(result, 1, 0, 0, 'WIRE', power: 0)
    end
  end
end
