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
        redpile(short ? '-v' : '--version').should == "Redpile #{REDPILE_VERSION}\n"
      end

      it 'prints a help message' do
        redpile(short ? '-h' : '--help').should =~ /^Redpile - High Performance Redstone/
      end

      it 'runs in interactive mode' do
        redpile(short ? '-i' : '--interactive').should == "\n"
      end

      it 'runs silently' do
        redpile(short ? '-s' : '--silent') do |p|
          p.puts 'SET 0 0 0 TORCH UP'
          p.puts 'SET 0 0 1 WIRE'
          p.puts 'TICK'
        end.should == "\n"
      end

      [1, 20, 2000].each do |size|
        it "runs with a custom world size of '#{size}'" do
          redpile("#{short ? '-w' : '--world-size'} #{size}").should == "\n"
        end
      end

      BAD_NUMBERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}")
          .should =~ /You must pass an integer as the world size\n/
        end
      end

      BAD_NEGATIVES.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}")
          .should =~ /You must provide a world size larger than zero\n/
        end
      end
    end
  end

  it 'runs in non interactive mode' do
    redpile.should == "\n"
  end

  it 'runs benchmarks' do
    redpile('--benchmark 1').should =~ /--- Benchmark Start ---\n/
  end

  BAD_NUMBERS.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile("--benchmark #{count}")
      .should =~ /You must pass an integer as the number of benchmarks to run\n/
    end
  end

  BAD_NEGATIVES.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile("--benchmark #{count}")
      .should =~ /You must provide a benchmark size greater than zero\n/
    end
  end
end
