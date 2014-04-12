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
  it 'parses the SET command' do
    redpile do |p|
      p.puts 'SET 0 0 0 EMPTY'
      p.close_write
      p.gets.should == "\n"
    end
  end

  it 'parses the GET command' do
    redpile do |p|
      p.puts 'GET 0 0 0'
      p.close_write
      p.gets.should == "(0,0,0) 0 EMPTY\n"
    end
  end

  it 'parses the TICK command' do
    redpile do |p|
      p.puts 'TICK'
      p.close_write
      p.gets.should == "\n"
    end
  end

  it 'error if given an incorrect material' do
    redpile do |p|
      p.puts 'SET 0 0 0 INVALID'
      p.close_write
      p.gets.should == "Invalid Command\n"
    end
  end


  normal_blocks = %w(AIR WIRE CONDUCTOR INSULATOR)
  normal_blocks.each do |block|
    it "inserts an #{block} block" do
      redpile do |p|
        p.puts "SET 0 0 0 #{block}"
        p.puts "GET 0 0 0"
        p.close_write
        p.gets.should == "(0,0,0) 0 #{block}\n"
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
          p.gets.should == "(0,0,0) 0 #{block} #{dir}\n"
        end
      end
    end
  end

  MAX_RANGE = 15
  [1, 32, 512, 1024].each do |size|
    context "with a world size of #{size}" do

      (1..MAX_RANGE).each do |range|
        it "propigates power #{range} blocks" do
          redpile("-w #{size}") do |p|
            p.puts 'SET 0 0 0 TORCH NORTH'
            (1..range).each {|r| p.puts "SET 0 0 #{r} WIRE"}
            p.puts 'TICK'
            p.close_write
            p.read.should =~ /\(0,0,#{range}\) #{16 - range}\n/
          end
        end
      end

      it "stops propigating power after #{MAX_RANGE} blocks" do
        end_block = MAX_RANGE + 1
        redpile("-w #{size}") do |p|
          p.puts 'SET 0 0 0 TORCH NORTH'
          (1..end_block).each {|r| p.puts "SET 0 0 #{r} WIRE"}
          p.puts 'TICK'
          p.close_write
          p.read.should =~ /\(0,0,#{end_block}\) 0\n/
        end
      end

      it 'Takes power from the closer torch' do
        redpile("-w #{size}") do |p|
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
        redpile("-w #{size}") do |p|
          p.puts 'SET 0 0 0 TORCH SOUTH'
          p.puts 'SET 0 0 -1 WIRE'
          p.puts 'SET 0 0 -2 TORCH UP'
          p.puts 'TICK'
          p.puts 'TICK'
          p.close_write
          p.read.should =~ /\(0,0,0\) 0\n/
        end
      end
    end
  end
end
