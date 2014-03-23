def redpile(opts = '')
  `./build/redpile #{opts}`
end

def redpile_version
  File.read('src/version.h')[/\d+\.\d+\.\d+/]
end

describe 'Redpile' do
  [true, false].each do |short|
    context "using #{short ? 'short' : 'long'} arguments" do

      it 'prints the current version' do
        redpile(short ? '-v' : '--version').should == "Redpile #{redpile_version}\n"
      end

      it 'prints a help message' do
        redpile(short ? '-h' : '--help').should =~ /^Redpile - High Performance Redstone/
      end

      it 'runs in interactive mode' do
        redpile(short ? '-i' : '--interactive').should == "Running in interactive mode\n";
      end

      it 'runs in normal mode' do
        redpile.should == "Running in normal mode\n";
      end
    end
  end
end
