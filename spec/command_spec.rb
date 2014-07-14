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

      it 'parses the SETR command' do
        run_case('SETR -5 -5 -5 5 5 5 TORCH UP', upper).should == "\n"
      end

      it 'parses the SETRS command' do
        run_case('SETRS -10 -10 -10 10 10 10 2 2 2 TORCH UP', upper).should == "\n"
      end

      it 'parses the GET command' do
        run_case('GET 0 0 0', upper).should =~ /^\(0,0,0\) AIR$/
      end

      it 'parses the TICK command' do
        run_case('TICK', upper).should == "\n"
      end

      it 'parses the TICKV command' do
        run_case('TICKV', upper).should =~ /^--- Tick 0 ---$/
      end

      it 'parses the TICKQ command' do
        run_case('TICKQ', upper).should == "\n"
      end

      it 'parses the STATUS command' do
        run_case('STATUS', upper).should =~ /^ticks: 0$/
      end

      it 'parses the PING command' do
        run_case('PING', upper).should =~ /^PONG$/
      end
    end
  end

  it 'errors on an unknown command' do
    run('INVALID').should =~ /^Unknown command 'INVALID'$/
  end

  it 'does not execute a commented out line' do
    run('# Comment goes here').should == "\n"
  end

  it 'error if given an incorrect type' do
    run('SET 0 0 0 INVALID').should =~ /^Unknown type: 'INVALID'$/
  end

  it 'errors if given an incorrect direction' do
    run('SET 0 0 0 TORCH INVALID').should =~ /^Unknown direction: 'INVALID'$/
  end

  it 'errors with a negative state' do
    run('SET 0 0 0 REPEATER NORTH -1').should =~ /^State must be non-negative$/
  end

  it 'errors with a state greater than 3' do
    run('SET 0 0 0 REPEATER NORTH 4').should =~ /^State must be less than three$/
  end

  it 'errors with a zero x step' do
    run('SETRS 0 0 0 10 10 10 0 2 2 WIRE').should =~ /^x_step must be greater than zero$/
  end

  it 'errors with a zero y step' do
    run('SETRS 0 0 0 10 10 10 2 0 2 WIRE').should =~ /^y_step must be greater than zero$/
  end

  it 'errors with a zero z step' do
    run('SETRS 0 0 0 10 10 10 2 2 0 WIRE').should =~ /^z_step must be greater than zero$/
  end

  it 'runs multiple ticks' do
    run('TICK 4', 'STATUS').should =~ /^ticks: 4$/
  end

  it 'errors for negative ticks' do
    run('TICK -2').should =~ /^Tick count must be greater than zero$/
  end

  it 'errors for non numerical ticks' do
    run('TICK abc').should =~ /^Tick count must be numeric$/
  end

  it 'adds a block' do
    run(
      'SET 0 0 0 AIR',
      'SET 0 0 1 WIRE',
      'STATUS'
    ).should =~ /^nodes: 2$/
  end

  it 'adds a block overlapping' do
    run(
      'SET 0 0 0 AIR',
      'SET 0 0 0 WIRE',
      'STATUS'
    ).should =~ /^nodes: 1$/
  end

  %w(AIR INSULATOR).each do |block|
    it "inserts an #{block} block" do
      run(
        "SET 0 0 0 #{block}",
        "GET 0 0 0"
      ).should =~ /^\(0,0,0\) #{block}$/
    end
  end

  %w(WIRE CONDUCTOR).each do |block|
    it "inserts an #{block} block" do
      run(
        "SET 0 0 0 #{block}",
        "GET 0 0 0"
      ).should =~ /^\(0,0,0\) #{block} 0$/
    end
  end

  %w(TORCH PISTON).each do |block|
    DIRECTIONS.each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        run(
          "SET 0 0 0 #{block} #{dir}",
          "GET 0 0 0"
        ).should =~ /^\(0,0,0\) #{block} 0 #{dir}$/
      end
    end
  end

  %w(REPEATER COMPARATOR SWITCH).each do |block|
    DIRECTIONS.each do |dir|
      it "inserts an #{block} block pointing #{dir}" do
        run(
          "SET 0 0 0 #{block} #{dir} 0",
          "GET 0 0 0"
        ).should =~ /^\(0,0,0\) #{block} 0 #{dir} 0$/
      end
    end
  end
end

