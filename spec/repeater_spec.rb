require 'spec_helper'
include Helpers

describe 'Repeater' do
  ['wire', 'conductor'].each do |material|
    it "passes power to a #{material}" do
      redpile do |p|
        p.puts 'SET 0 0 0 TORCH UP'
        p.puts 'SET 0 0 1 REPEATER SOUTH 0'
        p.puts "SET 0 0 2 #{material}"
        p.puts 'TICK'
        p.puts 'TICK'
      end.should =~ /\(0,0,2\) 15/
    end
  end

  (1..4).each do |delay|
    it "delays the propigation of power by #{delay} tick(s)" do
      redpile do |p|
        p.puts 'SET 0 0 0 TORCH UP'
        p.puts "SET 0 0 1 REPEATER SOUTH #{delay - 1}"
        p.puts 'SET 0 0 2 WIRE'
        delay.times { p.puts 'TICK' }
        p.puts 'GET 0 0 2'
        p.puts 'TICK'
        p.puts 'GET 0 0 2'
      end.should =~ /\(0,0,2\) 0 WIRE.*\(0,0,2\) 15 WIRE/m
    end
  end

  it 'is blocked from being powered by a repeater on the left' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET -1 0 2 TORCH UP'
      p.puts 'SET 0 0 1 REPEATER SOUTH 0'
      p.puts 'SET 0 0 2 REPEATER EAST 0'
      p.puts 'SET 1 0 2 WIRE'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET 1 0 2'
    end.should =~ /\(1,0,2\) 0 WIRE/m
  end

  it 'is blocked from being powered by a repeater on the right' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 1 0 2 TORCH UP'
      p.puts 'SET 0 0 1 REPEATER SOUTH 0'
      p.puts 'SET 0 0 2 REPEATER WEST 0'
      p.puts 'SET -1 0 2 WIRE'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET -1 0 2'
    end.should =~ /\(-1,0,2\) 0 WIRE/m
  end
end
