require 'spec_helper'
include Helpers

DIRECTIONS = %w(NORTH SOUTH EAST WEST UP DOWN)

def run_case(cmd, upper)
  run(cmd.send(upper ? :upcase : :downcase))
end

describe 'Commands' do
  [true, false].each do |upper|
    context "Using #{upper ? 'upper' : 'lower'} case" do
      it 'parses the SET command' do
        run_case('SET 0 0 0 TORCH UP', upper).should == "\n"
      end

      it 'parses the GET command' do
        run_case('GET 0 0 0', upper).should =~ /\(0,0,0\) 0 EMPTY/
      end

      it 'parses the TICK command' do
        run_case('TICK', upper).should == "\n"
      end

      it 'parses the STATUS command' do
        run_case('STATUS', upper).should =~ /ticks: 0/
      end

      it 'parses the PING command' do
        run_case('PING', upper).should =~ /PONG\n/
      end
    end
  end

  it 'does not execute a commented out line' do
    run('# Comment goes here').should == "\n"
  end

  it 'error if given an incorrect material' do
    run('SET 0 0 0 INVALID').should =~ /Invalid Command\n/
  end

  it 'errors if given an incorrect direction' do
    run('SET 0 0 0 TORCH INVALID').should =~ /Invalid Command\n/
  end

  it 'errors with a negative state' do
    run('SET 0 0 0 REPEATER NORTH -1').should =~ /Invalid Command\n/
  end

  it 'errors with a state greater than 3' do
    run('SET 0 0 0 REPEATER NORTH 4').should =~ /Invalid Command\n/
  end

  it 'runs multiple ticks' do
    run('TICK 4', 'STATUS').should =~ /ticks: 4\n/
  end

  it 'does not run negative ticks' do
    run('TICK -2', 'STATUS').should =~ /ticks: 0\n/
  end

  it 'errors for non numerical ticks' do
    run('TICK abc', 'STATUS').should =~ /Invalid Command\n/
  end

  it 'adds a block' do
    run(
      'SET 0 0 0 AIR',
      'SET 0 0 1 WIRE',
      'STATUS'
    ).should =~ /blocks: 2\n/
  end

  it 'adds a block overlapping' do
    run(
      'SET 0 0 0 AIR',
      'SET 0 0 0 WIRE',
      'STATUS'
    ).should =~ /blocks: 1\n/
  end

  it 'removes a block' do
    run(
      'SET 0 0 0 AIR',
      'SET 0 0 1 WIRE',
      'SET 0 0 0 EMPTY',
      'STATUS'
    ).should =~ /blocks: 1\n/
  end

  it "doesn't add an empty block" do
    run(
      'SET 0 0 0 EMPTY',
      'STATUS'
    ).should =~ /blocks: 0\n/
  end

  %w(AIR WIRE CONDUCTOR INSULATOR).each do |block|
    it "inserts an #{block} block" do
      run(
        "SET 0 0 0 #{block}",
        "GET 0 0 0"
      ).should =~ /\(0,0,0\) 0 #{block}\n/
    end
  end

  %w(TORCH PISTON).each do |block|
    DIRECTIONS.each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        run(
          "SET 0 0 0 #{block} #{dir}",
          "GET 0 0 0"
        ).should =~ /\(0,0,0\) 0 #{block} #{dir}\n/
      end
    end
  end

  %w(REPEATER COMPARATOR SWITCH).each do |block|
    DIRECTIONS.each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        run(
          "SET 0 0 0 #{block} #{dir} 0",
          "GET 0 0 0"
        ).should =~ /\(0,0,0\) 0 #{block} #{dir} 0\n/
      end
    end
  end
end

