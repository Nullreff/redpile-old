require 'timeout'

def redpile(opts = '', &block)
  IO.popen("./build/redpile #{opts}", 'r+', &block)
end

def redpile_version
  File.read('src/version.h')[/\d+\.\d+\.\d+/]
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
      p.puts 'SET 0 0 0 1'
      p.close_write
      p.gets.should == "(0,0,0) AIR 0\n"
    end
  end

  it 'parses the POWER command' do
    redpile do |p|
      p.puts 'SET 0 0 0 2'
      p.puts 'POWER 0 0 0 15'
      p.close_write
      p.gets # SET
      p.gets.should == "(0,0,0) WIRE 15\n"
    end
  end

  it 'parses the GET command' do
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

  it 'parses the TICK command' do
    redpile do |p|
      p.puts 'TICK'
      p.close_write
      p.gets.should == "Not implemented...\n"
    end
  end
end
