# command_spec.rb - Command tests for Redpile
#
# Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

require 'spec_helper'
include Helpers

describe 'Redpile Commands' do
  [true, false].each do |upper|
    context "Using #{upper ? 'upper' : 'lower'} case" do
      it 'parses the SET command' do
        redpile do |p|
          p.puts (upper ? 'SET 0 0 0 TORCH UP' : 'set 0 0 0 torch up')
          p.close_write
          p.read.should == "\n"
        end
      end

      it 'parses the GET command' do
        redpile do |p|
          p.puts (upper ? 'GET 0 0 0' : 'get 0 0 0')
          p.close_write
          p.read.should =~ /\(0,0,0\) 0 EMPTY/
        end
      end

      it 'parses the TICK command' do
        redpile do |p|
          p.puts (upper ? 'TICK' : 'tick')
          p.close_write
          p.read.should == "\n"
        end
      end

      it 'parses the STATUS command' do
        redpile do |p|
          p.puts (upper ? 'STATUS' : 'status')
          p.close_write
          p.read.should =~ /ticks: 0/
        end
      end

      it 'parses the PING command' do
        redpile do |p|
          p.puts (upper ? 'PING' : 'ping')
          p.close_write
          p.read.should =~ /PONG\n/
        end
      end
    end
  end

  it 'error if given an incorrect material' do
    redpile do |p|
      p.puts 'SET 0 0 0 INVALID'
      p.close_write
      p.read.should =~ /Invalid Command\n/
    end
  end

  it 'errors if given an incorrect direction' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH INVALID'
      p.close_write
      p.gets.should =~ /Invalid Command\n/
    end
  end

  it 'errors with a negative state' do
    redpile do |p|
      p.puts 'SET 0 0 0 REPEATER NORTH -1'
      p.close_write
      p.gets.should =~ /Invalid Command\n/
    end
  end

  it 'adds a block' do
    redpile do |p|
      p.puts 'SET 0 0 0 AIR'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'STATUS'
      p.close_write
      p.read.should =~ /blocks: 2/
    end
  end

  it 'adds a block overlapping' do
    redpile do |p|
      p.puts 'SET 0 0 0 AIR'
      p.puts 'SET 0 0 0 WIRE'
      p.puts 'STATUS'
      p.close_write
      p.read.should =~ /blocks: 1/
    end
  end

  it 'removes a block' do
    redpile do |p|
      p.puts 'SET 0 0 0 AIR'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 0 EMPTY'
      p.puts 'STATUS'
      p.close_write
      p.read.should =~ /blocks: 1/
    end
  end

  it 'doesn\'t add an empty block' do
    redpile do |p|
      p.puts 'SET 0 0 0 EMPTY'
      p.puts 'STATUS'
      p.close_write
      p.read.should =~ /blocks: 0/
    end
  end

  normal_blocks = %w(AIR WIRE CONDUCTOR INSULATOR)
  normal_blocks.each do |block|
    it "inserts an #{block} block" do
      redpile do |p|
        p.puts "SET 0 0 0 #{block}"
        p.puts "GET 0 0 0"
        p.close_write
        p.read.should =~ /\(0,0,0\) 0 #{block}\n/
      end
    end
  end

  direction_blocks = %w(TORCH)
  direction_blocks.each do |block|
    %w(NORTH SOUTH EAST WEST UP DOWN).each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        redpile do |p|
          p.puts "SET 0 0 0 #{block} #{dir}"
          p.puts "GET 0 0 0"
          p.close_write
          p.read.should =~ /\(0,0,0\) 0 #{block} #{dir}\n/
        end
      end
    end
  end

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

  it 'Takes power from the closer torch' do
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

  it 'Turns a torch off with power' do
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

  it 'follows wires over and down' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 AIR'
      p.puts 'SET 0 -1 2 WIRE'
      p.puts 'TICK'
      p.close_write
      p.read.should =~ /\(0,-1,2\) 14/
    end
  end

  it 'follows wires over and up' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 1 2 WIRE'
      p.puts 'TICK'
      p.close_write
      p.read.should =~ /\(0,1,2\) 14/
    end
  end

  it 'does not propigate over and up when a block is on top' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 1 2 WIRE'
      p.puts 'SET 0 1 1 CONDUCTOR'
      p.puts 'TICK'
      p.close_write
      p.read.should_not =~ /\(0,1,2\)/
    end
  end

  it 'powers an adjacent conductor from wire' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
      p.close_write
      p.read.should =~ /\(0,0,2\) 15/
    end
  end

  it 'diverts power from a condutor to a wire on the left' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
      p.close_write
      r = p.read
      r.should_not =~ /\(0,0,2\)/
      r.should =~ /\(1,0,1\) 14/
    end
  end

  it 'diverts power from a condutor to a wire on the right' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
      p.close_write
      r = p.read
      r.should_not =~ /\(0,0,2\)/
      r.should =~ /\(-1,0,1\) 14/
    end
  end

  it 'diverts power from a condutor to a wire on both sides' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
      p.close_write
      r = p.read
      r.should_not =~ /\(0,0,2\)/
      r.should =~ /\(1,0,1\) 14/
      r.should =~ /\(-1,0,1\) 14/
    end
  end

  it 'does not divert power from a conductor if there is a wire on top' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 1 2 WIRE'
      p.puts 'TICK'
      p.close_write
      r = p.read
      r.should =~ /\(0,0,2\) 14/
      r.should =~ /\(0,1,2\) 14/
      r.should =~ /\(1,0,1\) 14/
      r.should =~ /\(-1,0,1\) 14/
    end
  end

  it 'propigates power to a conductor underneath wire' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 -1 1 CONDUCTOR'
      p.puts 'TICK'
      p.close_write
      p.read.should =~ /\(0,-1,1\) 15/
    end
  end

  ['wire', 'conductor'].each do |material|
    it "passes power through a repeater to a #{material}" do
      redpile do |p|
        p.puts 'SET 0 0 0 TORCH UP'
        p.puts 'SET 0 0 1 REPEATER SOUTH 0'
        p.puts "SET 0 0 2 #{material}"
        p.puts 'TICK'
        p.puts 'TICK'
        p.close_write
        p.read.should =~ /\(0,0,2\) 15/
      end
    end
  end

  (0..3).each do |delay|
    it "delays the propigation of power by #{delay} tick(s)" do
      redpile do |p|
        p.puts 'SET 0 0 0 TORCH UP'
        p.puts 'TICK'
        p.puts "SET 0 0 1 REPEATER SOUTH #{delay}"
        p.puts 'SET 0 0 2 WIRE'
        delay.times { p.puts 'TICK' }
        p.puts 'GET 0 0 2'
        p.puts 'TICK'
        p.puts 'GET 0 0 2'
        p.close_write
        p.read.should =~ /\(0,0,2\) 0 WIRE.*\(0,0,2\) 15 WIRE/m
      end
    end
  end
end
