require 'spec_helper'
include Helpers

describe 'Comparator' do
  it 'allows power through if side power is less than the input' do
    result = redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 WIRE'
      p.puts 'SET 1 0 3 TORCH UP'
      p.puts 'SET 0 0 3 COMPARATOR WEST 0'
      p.puts 'SET -1 0 3 WIRE'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET -1 0 3'
    end
    result.should =~ /\(-1,0,3\) 15 WIRE/m
  end

  it 'does not allow power through if side power is greater than the input' do
    result = redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 2 0 1 TORCH UP'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET 0 0 1 COMPARATOR WEST 0'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET -1 0 1'
    end
    result.should =~ /\(-1,0,1\) 0 WIRE/m
  end
end
