require 'spec_helper'
include Helpers

describe 'Redpile Commands' do
  [true, false].each do |upper|
    context "Using #{upper ? 'upper' : 'lower'} case" do
      it 'parses the SET command' do
        redpile do |p|
          p.puts(upper ? 'SET 0 0 0 TORCH UP' : 'set 0 0 0 torch up')
        end.should == "\n"
      end

      it 'parses the GET command' do
        redpile do |p|
          p.puts(upper ? 'GET 0 0 0' : 'get 0 0 0')
        end.should =~ /\(0,0,0\) 0 EMPTY/
      end

      it 'parses the TICK command' do
        redpile do |p|
          p.puts(upper ? 'TICK' : 'tick')
        end.should == "\n"
      end

      it 'parses the STATUS command' do
        redpile do |p|
          p.puts(upper ? 'STATUS' : 'status')
        end.should =~ /ticks: 0/
      end

      it 'parses the PING command' do
        redpile do |p|
          p.puts(upper ? 'PING' : 'ping')
        end.should =~ /PONG\n/
      end
    end
  end

  it 'does not execute a commented out line' do
    redpile do |p|
      p.puts '# Comment goes here'
    end.should == "\n"
  end

  it 'error if given an incorrect material' do
    redpile do |p|
      p.puts 'SET 0 0 0 INVALID'
    end.should =~ /Invalid Command\n/
  end

  it 'errors if given an incorrect direction' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH INVALID'
    end.should =~ /Invalid Command\n/
  end

  it 'errors with a negative state' do
    redpile do |p|
      p.puts 'SET 0 0 0 REPEATER NORTH -1'
    end.should =~ /Invalid Command\n/
  end

  it 'errors with a state greater than 3' do
    redpile do |p|
      p.puts 'SET 0 0 0 REPEATER NORTH 4'
    end.should =~ /Invalid Command\n/
  end

  it 'runs multiple ticks' do
    redpile do |p|
      p.puts 'TICK 4'
      p.puts 'STATUS'
    end.should =~ /ticks: 4\n/
  end

  it 'does not run negative ticks' do
    redpile do |p|
      p.puts 'TICK -2'
      p.puts 'STATUS'
    end.should =~ /ticks: 0\n/
  end

  it 'errors for non numerical ticks' do
    redpile do |p|
      p.puts 'TICK abc'
      p.puts 'STATUS'
    end.should =~ /Invalid Command\n/
  end

  it 'adds a block' do
    redpile do |p|
      p.puts 'SET 0 0 0 AIR'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'STATUS'
    end.should =~ /blocks: 2/
  end

  it 'adds a block overlapping' do
    redpile do |p|
      p.puts 'SET 0 0 0 AIR'
      p.puts 'SET 0 0 0 WIRE'
      p.puts 'STATUS'
    end.should =~ /blocks: 1/
  end

  it 'removes a block' do
    redpile do |p|
      p.puts 'SET 0 0 0 AIR'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 0 EMPTY'
      p.puts 'STATUS'
    end.should =~ /blocks: 1/
  end

  it "doesn't add an empty block" do
    redpile do |p|
      p.puts 'SET 0 0 0 EMPTY'
      p.puts 'STATUS'
    end.should =~ /blocks: 0/
  end

  normal_blocks = %w(AIR WIRE CONDUCTOR INSULATOR)
  normal_blocks.each do |block|
    it "inserts an #{block} block" do
      redpile do |p|
        p.puts "SET 0 0 0 #{block}"
        p.puts "GET 0 0 0"
      end.should =~ /\(0,0,0\) 0 #{block}\n/
    end
  end

  direction_blocks = %w(TORCH PISTON)
  direction_blocks.each do |block|
    %w(NORTH SOUTH EAST WEST UP DOWN).each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        redpile do |p|
          p.puts "SET 0 0 0 #{block} #{dir}"
          p.puts "GET 0 0 0"
        end.should =~ /\(0,0,0\) 0 #{block} #{dir}\n/
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
        end.should =~ /\(0,0,0\) 0 #{block} #{dir} 0\n/
      end
    end
  end
end
