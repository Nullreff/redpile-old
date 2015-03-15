require 'spec_helper'
include Helpers

REDPILE_VERSION = File.read('src/redpile.rs')[/^static REDPILE_VERSION.+?"(\d+\.\d+\.\d+)"/, 1]
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
        redpile(short ? '-h' : '--help').run.should =~ /^Redpile - A Voxel Logic Simulator/
      end

      [1, 32, 1024].each do |size|
        it "runs with a custom world size of '#{size}'" do
          redpile("#{short ? '-w' : '--world-size'} #{size}").run.should == ''
        end
      end

      BAD_NUMBERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(opts: short ? "-w #{size}" : "--world-size #{size}", result: EXIT_FAILURE).
          run.should == 'You must pass a power of two between 1 and 4294967295 as the world size'
        end
      end

      BAD_NEGATIVES.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(opts: short ? "-w #{size}" : "--world-size #{size}", result: EXIT_FAILURE).
          run.should == 'You must pass a power of two between 1 and 4294967295 as the world size'
        end
      end

      BAD_POWERS.each do |size|
        it "errors when run with a world size of '#{size}'" do
          redpile(opts: short ? "-w #{size}" : "--world-size #{size}", result: EXIT_FAILURE).
          run.should == 'You must pass a power of two between 1 and 4294967295 as the world size'
        end
      end

      BAD_NUMBERS.each do |port|
        it "errors when run on the port #{port}" do
          redpile(opts: short ? "-p #{port}" : "--port #{port}", result: EXIT_FAILURE).
          run.should == 'You must pass a number between 1 and 65535 as the port number'
        end
      end

      BAD_NEGATIVES.each do |port|
        it "errors when run on the port #{port}" do
          redpile(opts: short ? "-p #{port}" : "--port #{port}", result: EXIT_FAILURE).
          run.should == 'You must pass a number between 1 and 65535 as the port number'
        end
      end

      BAD_PORTS.each do |port|
        it "errors when run on the port #{port}" do
          redpile(opts: short ? "-p #{port}" : "--port #{port}", result: EXIT_FAILURE).
          run.should == 'You must pass a number between 1 and 65535 as the port number'
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
      run.should == 'You must pass a positive number of seconds to run each benchmark for'
    end
  end

  BAD_NEGATIVES.each do |count|
    it "errors when run with '#{count}' benchmarks" do
      redpile(opts: "--benchmark #{count}", result: EXIT_FAILURE).
      run.should == 'You must pass a positive number of seconds to run each benchmark for'
    end
  end

  it 'errors when given an empty configuration file' do
    redpile(config: '/dev/null', result: EXIT_FAILURE).
    run.should == 'No types defined in configuration file /dev/null'
  end
end

