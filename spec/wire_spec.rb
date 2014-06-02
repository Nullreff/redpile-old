require 'spec_helper'
include Helpers

describe 'Wire' do
  it 'propigates power to the side and down' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 AIR'
      p.puts 'SET 0 -1 2 WIRE'
      p.puts 'TICK'
    end.should =~ /\(0,-1,2\) 14/
  end

  it 'propigates power to the side and up' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 1 2 WIRE'
      p.puts 'TICK'
    end.should =~ /\(0,1,2\) 14/
  end

  it 'does not propigate to the side and up when a block is on top' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 1 2 WIRE'
      p.puts 'SET 0 1 1 CONDUCTOR'
      p.puts 'TICK'
    end.should_not =~ /\(0,1,2\)/
  end

  it 'powers an adjacent conductor' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
    end.should =~ /\(0,0,2\) 15/
  end

  it 'diverts power from a condutor when a wire is on the left' do
    result = redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
    end
    result.should_not =~ /\(0,0,2\)/
    result.should =~ /\(1,0,1\) 14/
  end

  it 'diverts power from a condutor when a wire is on the right' do
    result = redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
    end
    result.should_not =~ /\(0,0,2\)/
    result.should =~ /\(-1,0,1\) 14/
  end

  it 'diverts power from a condutor when a wire is on both sides' do
    result = redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'TICK'
    end
    result.should_not =~ /\(0,0,2\)/
    result.should =~ /\(1,0,1\) 14/
    result.should =~ /\(-1,0,1\) 14/
  end

  it 'does not divert power from a conductor if there is a wire on top' do
    result = redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 1 0 1 WIRE'
      p.puts 'SET -1 0 1 WIRE'
      p.puts 'SET 0 0 2 CONDUCTOR'
      p.puts 'SET 0 1 2 WIRE'
      p.puts 'TICK'
    end
    result.should =~ /\(0,0,2\) 14/
    result.should =~ /\(0,1,2\) 14/
    result.should =~ /\(1,0,1\) 14/
    result.should =~ /\(-1,0,1\) 14/
  end

  it 'propigates power to a conductor underneath wire' do
    redpile do |p|
      p.puts 'SET 0 0 0 TORCH UP'
      p.puts 'SET 0 0 1 WIRE'
      p.puts 'SET 0 -1 1 CONDUCTOR'
      p.puts 'TICK'
    end.should =~ /\(0,-1,1\) 15/
  end
end
