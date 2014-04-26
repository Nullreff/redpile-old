# cil_spec.rb - CLI tests for Redpile
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

require 'timeout'
require 'spec_helper'
include Helpers

REDPILE_VERSION = File.read('src/redpile.h')[/REDPILE_VERSION "(\d+\.\d+\.\d+)"/, 1]
BAD_NEGATIVES = [0, -1, -20]
BAD_NUMBERS = ['abc', 'a12', '12c']

describe 'Redpile Options' do
  [true, false].each do |short|
    context "using #{short ? 'short' : 'long'} arguments" do

      it 'prints the current version' do
        redpile(short ? '-v' : '--version') {|p| p.gets.should == "Redpile #{REDPILE_VERSION}\n" }
      end

      it 'prints a help message' do
        redpile(short ? '-h' : '--help') {|p| p.gets.should =~ /^Redpile - High Performance Redstone/ }
      end

      it 'runs in interactive mode' do
        redpile(short ? '-i' : '--interactive') do |p|
          p.close_write
          p.read.should == ""
        end
      end

      it 'runs silently' do
        redpile(short ? '-s' : '--silent') do |p|
          p.puts 'SET 0 0 0 TORCH UP'
          p.puts 'SET 0 0 1 WIRE'
          p.puts 'TICK'
          p.close_write
          p.read.should == ""
        end
      end

      [1, 20, 2000].each do |size|
        it "runs with a custom world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}") do |p|
            p.close_write
            p.read.should == ""
          end
        end
      end

      BAD_NUMBERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}") do |p|
            p.close_write
            p.read.should =~ /You must pass an integer as the world size\n/
          end
        end
      end

      BAD_NEGATIVES.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}") do |p|
            p.close_write
            p.read.should =~ /You must provide a world size larger than zero\n/
          end
        end
      end
    end
  end

  it 'runs in non interactive mode' do
    redpile do |p|
      p.close_write
      p.read.should == ""
    end
  end

  it 'runs benchmarks' do
    redpile('--benchmark 1') do |p|
      p.close_write
      p.read.should =~ /--- Benchmark Start ---\n/
    end
  end

  BAD_NUMBERS.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile("--benchmark #{count}") do |p|
        p.close_write
        p.read.should =~ /You must pass an integer as the number of benchmarks to run\n/
      end
    end
  end

  BAD_NEGATIVES.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile("--benchmark #{count}") do |p|
        p.close_write
        p.read.should =~ /You must provide a benchmark size greater than zero\n/
      end
    end
  end
end
