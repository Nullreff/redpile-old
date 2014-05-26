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

  direction_blocks = %w(TORCH PISTON)
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

  state_blocks = %w(REPEATER COMPARATOR)
  state_blocks.each do |block|
    %w(NORTH SOUTH EAST WEST UP DOWN).each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        redpile do |p|
          p.puts "SET 0 0 0 #{block} #{dir} 0"
          p.puts "GET 0 0 0"
          p.close_write
          p.read.should =~ /\(0,0,0\) 0 #{block} #{dir} 0\n/
        end
      end
    end
  end
end
