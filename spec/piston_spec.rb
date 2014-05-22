require 'spec_helper'
include Helpers

describe 'Piston' do
  it 'extends a conductor when powered' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 PISTON SOUTH 0'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 0 3 AIR'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET 0 0 2'
      p.puts 'GET 0 0 3'
      p.close_write
      p.read.should =~ /\(0,0,2\) 0 INSULATOR.*\(0,0,3\) 0 CONDUCTOR/m
    end
  end

  it 'extends a conductor into a empty block' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 PISTON SOUTH 0'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET 0 0 2'
      p.puts 'GET 0 0 3'
      p.close_write
      p.read.should =~ /\(0,0,2\) 0 INSULATOR.*\(0,0,3\) 0 CONDUCTOR/m
    end
  end

  it 'retracts a condutor when unpowered' do
    redpile do |p|
      p.puts 'SET 0 0 1 PISTON SOUTH 1'
      p.puts 'SET 0 0 2 INSULATOR'
      p.puts 'SET 0 0 3 CONDUCTOR'
      p.puts 'TICK'
      p.puts 'GET 0 0 2'
      p.puts 'GET 0 0 3'
      p.close_write
      p.read.should =~ /\(0,0,2\) 0 CONDUCTOR.*\(0,0,3\) 0 AIR/m
    end
  end

  it 'extends and retracts a conductor' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 PISTON SOUTH 0'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 0 3 AIR'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'SET 0 0 0 EMPTY'
      p.puts 'TICK'
      p.puts 'TICK'
      p.puts 'GET 0 0 2'
      p.puts 'GET 0 0 3'
      p.close_write
      p.read.should =~ /\(0,0,2\) 0 CONDUCTOR.*\(0,0,3\) 0 AIR/m
    end
  end

end
