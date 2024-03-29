require 'spec_helper'
include Helpers

describe 'Commands' do
  it 'parses the PING command' do
    run('PING').should == 'PONG'
  end

  it 'parses the DELETE command' do
    run('DELETE 0,0,0').should == ''
  end

  it 'errors on an unknown command' do
    run('INVALID').should == "Unknown command 'INVALID'"
  end

  it 'does not execute a commented out line' do
    run('# Comment goes here').should == ''
  end

  it 'ignores a comment on a line with a command' do
    run('PING # Comment goes here').should == 'PONG'
  end
end

