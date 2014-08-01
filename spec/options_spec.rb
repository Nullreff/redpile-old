require 'timeout'
require 'spec_helper'
include Helpers

REDPILE_VERSION = File.read('src/redpile.h')[/REDPILE_VERSION "(\d+\.\d+\.\d+)"/, 1]
BAD_NEGATIVES = [0, -1, -20]
BAD_NUMBERS = ['abc', 'a12', '12c']
BAD_POWERS = [3, 13, 28]
BAD_PORTS = [65536, 100000000]

describe 'Options' do
  [true, false].each do |short|
    context "using #{short ? 'short' : 'long'} arguments" do

      it 'prints the current version' do
        redpile(short ? '-v' : '--version').run.should == "Redpile #{REDPILE_VERSION}\n"
      end

      it 'prints a help message' do
        redpile(short ? '-h' : '--help').run.should =~ /^Redpile - High Performance Redstone/
      end

      it 'runs in interactive mode' do
        redpile(short ? '-i' : '--interactive').run.should == "\n"
      end

      [1, 32, 1024].each do |size|
        it "runs with a custom world size of '#{size}'" do
          redpile("#{short ? '-w' : '--world-size'} #{size}").run.should == "\n"
        end
      end

      BAD_NUMBERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}", false).
          run.should =~ /You must pass an integer as the world size\n/
        end
      end

      BAD_NEGATIVES.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}", false).
          run.should =~ /You must provide a world size larger than zero\n/
        end
      end

      BAD_POWERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(short ? "-w #{size}" : "--world-size #{size}", false).
          run.should =~ /You must provide a world size that is a power of two\n/
        end
      end

      BAD_NUMBERS.each do |port|
        it "errors when run on the port #{port}" do
          redpile(short ? "-p #{port}" : "--port #{port}", false).
          run.should =~ /You must pass an integer as the port number\n/
        end
      end

      BAD_NEGATIVES.each do |port|
        it "errors when run on the port #{port}" do
          redpile(short ? "-p #{port}" : "--port #{port}", false).
          run.should =~ /You must provide a port number greater than zero\n/
        end
      end

      BAD_PORTS.each do |port|
        it "errors when run on the port #{port}" do
          redpile(short ? "-p #{port}" : "--port #{port}", false).
          run.should =~ /You must provide a port number less than or equal to 65535\n/
        end
      end
    end
  end

  it 'runs in non interactive mode' do
    redpile.run.should == "\n"
  end

  it 'runs benchmarks' do
    redpile('--benchmark 1').run.should =~ /--- Benchmark Start ---\n/
  end

  BAD_NUMBERS.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile("--benchmark #{count}", false).
      run.should =~ /You must pass an integer as the number of benchmarks to run\n/
    end
  end

  BAD_NEGATIVES.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile("--benchmark #{count}", false).
      run.should =~ /You must provide a benchmark size greater than zero\n/
    end
  end
end

