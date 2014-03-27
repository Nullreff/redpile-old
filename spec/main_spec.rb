require 'timeout'

def redpile(opts = '', &block)
  IO.popen("./build/redpile #{opts}", 'r+', &block)
end

def redpile_version
  File.read('src/redpile.h')[/REDPILE_VERSION "(\d+\.\d+\.\d+)"/, 1]
end

def blocks
  {'AIR' => 1, 'WIRE' => 2, 'CONDUCTOR' => 3, 'INSULATOR' => 4}
end

describe 'Redpile' do
  [true, false].each do |short|
    context "using #{short ? 'short' : 'long'} arguments" do

      it 'prints the current version' do
        redpile(short ? '-v' : '--version') {|p| p.gets.should == "Redpile #{redpile_version}\n" }
      end

      it 'prints a help message' do
        redpile(short ? '-h' : '--help') {|p| p.gets.should =~ /^Redpile - High Performance Redstone/ }
      end

      it 'runs in interactive mode' do
        redpile(short ? '-i' : '--interactive') do |p|
          p.close_write
          p.gets.should == "> \n"
        end
      end
    end
  end

  it 'runs in non interactive mode' do
    redpile do |p|
      p.close_write
      p.gets.should == "\n"
    end
  end

  it 'parses the SET command' do
    redpile do |p|
      p.puts 'SET 0 0 0 0'
      p.close_write
      p.gets.should == "(0,0,0) EMPTY 0\n"
    end
  end

  it 'parses the POWER command' do
    redpile do |p|
      p.puts 'POWER 0 0 0 15'
      p.close_write
      p.gets.should == "\n"
    end
  end

  it 'parses the GET command' do
    redpile do |p|
      p.puts 'GET 0 0 0'
      p.close_write
      p.gets.should == "(0,0,0) EMPTY 0\n"
    end
  end

  it 'parses the TICK command' do
    redpile do |p|
      p.puts 'TICK'
      p.close_write
      p.gets.should == "\n"
    end
  end

  blocks.each do |block, num|
    it "inserts an #{block} block" do
      redpile do |p|
        p.puts "SET 0 0 0 #{num}"
        p.close_write
        p.gets.should == "(0,0,0) #{block} 0\n"
      end
    end
  end

  it 'retreives the power and material of a block' do
    redpile do |p|
      p.puts 'SET 0 0 0 2'
      p.puts 'POWER 0 0 0 15'
      p.puts 'GET 0 0 0'
      p.close_write
      p.gets # SET
      p.gets # POWER
      p.gets.should == "(0,0,0) WIRE 15\n"
    end
  end

  it 'set the power and material of a block' do
    redpile do |p|
      p.puts 'SET 0 0 0 2'
      p.puts 'POWER 0 0 0 15'
      p.close_write
      p.gets # SET
      p.gets.should == "(0,0,0) WIRE 15\n"
    end
  end

  it 'deincrements the power of a block on tick' do
    redpile do |p|
      p.puts 'SET 0 0 0 2'
      p.puts 'POWER 0 0 0 15'
      p.puts 'TICK'
      p.close_write
      p.gets # SET
      p.gets # POWER
      p.gets.should == "(0,0,0) WIRE 14\n"
    end
  end
end
