require 'spec_helper'
include Helpers

describe 'Torch' do
  MAX_RANGE = 15
  (1..MAX_RANGE).each do |range|
    it "propigates power #{range} blocks" do
      redpile do |p|
        p.puts 'SET 0 0 0 TORCH UP'
        (1..range).each {|r| p.puts "SET 0 0 #{r} WIRE"}
        p.puts 'TICK'
        p.close_write
        p.read.should =~ /\(0,0,#{range}\) #{16 - range}\n/
      end
    end
  end

  it "stops propigating power after #{MAX_RANGE} blocks" do
    end_block = MAX_RANGE + 1
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      (1..end_block).each {|r| p.puts "SET 0 0 #{r} WIRE"}
      p.puts 'TICK'
      p.puts "GET 0 0 #{end_block}"
      p.close_write
      p.read.should =~ /\(0,0,#{end_block}\) 0 WIRE\n/
    end
  end

  it 'takes power from the closer torch' do
    redpile do |p|
      p.puts 'SET 0 0 -3 TORCH NORTH'
      p.puts 'SET 0 0 -2 WIRE'
      p.puts 'SET 0 0 -1 WIRE'
      p.puts 'SET 0 0 0 WIRE'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 TORCH NORTH'
      p.puts 'TICK'
      p.close_write
      p.read.should =~ /\(0,0,0\) 14\n/
    end
  end

  it 'turns a torch off with power' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH SOUTH'
      p.puts 'SET 0 0 -1 WIRE'
      p.puts 'SET 0 0 -2 TORCH UP'
      p.puts 'TICK'
      p.puts 'TICK'
      p.close_write
      p.read.should =~ /\(0,0,0\) 0\n/
    end
  end

  it 'passes power up through a conductor' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 1 0 CONDUCTOR'
      p.puts 'SET 0 2 0 WIRE'
      p.puts 'TICK'
      p.close_write
      r = p.read
      r.should =~ /\(0,1,0\) 15/
      r.should =~ /\(0,2,0\) 15/
    end
  end
end
