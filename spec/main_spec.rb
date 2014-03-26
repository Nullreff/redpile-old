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
    end
  end
end
