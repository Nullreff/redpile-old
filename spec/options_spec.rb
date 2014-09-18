require 'spec_helper'
include Helpers

REDPILE_VERSION = File.read('src/redpile.h')[/REDPILE_VERSION "(\d+\.\d+\.\d+)"/, 1]
BAD_NEGATIVES = [0, -1, -20]
BAD_NUMBERS = ['abc', 'a12', '12c']
BAD_POWERS = [3, 13, 28]
BAD_PORTS = [65536, 100000000]
EXIT_FAILURE = 256

describe 'Options' do
  [true, false].each do |short|
    context "using #{short ? 'short' : 'long'} arguments" do
      it 'prints the current version' do
        redpile(short ? '-v' : '--version').run.should == "Redpile #{REDPILE_VERSION}"
      end

      it 'prints a help message' do
        redpile(short ? '-h' : '--help').run.should =~ /^Redpile - High Performance Redstone/
      end

      it 'runs in interactive mode' do
        redpile(short ? '-i' : '--interactive').run.should == ''
      end

      [1, 32, 1024].each do |size|
        it "runs with a custom world size of '#{size}'" do
          redpile("#{short ? '-w' : '--world-size'} #{size}").run.should == ''
        end
      end

      BAD_NUMBERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(opts: short ? "-w #{size}" : "--world-size #{size}", result: EXIT_FAILURE).
          run.should == 'You must pass an integer as the world size'
        end
      end

      BAD_NEGATIVES.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(opts: short ? "-w #{size}" : "--world-size #{size}", result: EXIT_FAILURE).
          run.should == 'You must provide a world size larger than zero'
        end
      end

      BAD_POWERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(opts: short ? "-w #{size}" : "--world-size #{size}", result: EXIT_FAILURE).
          run.should == 'You must provide a world size that is a power of two'
        end
      end

      BAD_NUMBERS.each do |port|
        it "errors when run on the port #{port}" do
          redpile(opts: short ? "-p #{port}" : "--port #{port}", result: EXIT_FAILURE).
          run.should == 'You must pass an integer as the port number'
        end
      end

      BAD_NEGATIVES.each do |port|
        it "errors when run on the port #{port}" do
          redpile(opts: short ? "-p #{port}" : "--port #{port}", result: EXIT_FAILURE).
          run.should == 'You must provide a port number greater than zero'
        end
      end

      BAD_PORTS.each do |port|
        it "errors when run on the port #{port}" do
          redpile(opts: short ? "-p #{port}" : "--port #{port}", result: EXIT_FAILURE).
          run.should == 'You must provide a port number less than or equal to 65535'
        end
      end
    end
  end

  it 'runs in non interactive mode' do
    redpile.run.should == ''
  end

  it 'runs benchmarks' do
    redpile('--benchmark 1').run.should =~ /^--- Benchmark Start ---$/
  end

  BAD_NUMBERS.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile(opts: "--benchmark #{count}", result: EXIT_FAILURE).
      run.should == 'You must pass an integer as the number of benchmarks to run'
    end
  end

  BAD_NEGATIVES.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile(opts: "--benchmark #{count}", result: EXIT_FAILURE).
      run.should == 'You must provide a benchmark size greater than zero'
    end
  end

  it 'errors when given an empty configuration file' do
    redpile(config: '/dev/null', result: EXIT_FAILURE).
    run.should == 'No types defined in configuration file /dev/null'
  end
end

