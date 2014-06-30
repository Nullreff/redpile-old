module Helpers
  REDPILE_CMD = './build/redpile'
  VALGRIND_CMD = 'valgrind -q --error-exitcode=1 --leak-check=full --show-reachable=yes'

  class Redpile
    def initialize(process, test_exit)
      @process = process
      @test_exit = test_exit
    end

    def run(*commands)
      commands.each {|cmd| @process.puts cmd}
      @process.close_write
      result = @process.read
      @process.close
      raise "Exited with status code #{$?.to_i}" if @test_exit && $?.to_i > 0
      result
    end
  end

  def redpile(opts = '', test_exit = ENV['VALGRIND'])
    process = IO.popen("#{ENV['VALGRIND'] ? VALGRIND_CMD : ''} #{REDPILE_CMD} #{opts} 2>&1", 'r+')
    Redpile.new(process, test_exit)
  end
  
  def run(*commands)
    redpile.run(commands)
  end

  def powered(x, y, z)
    /^\(#{x},#{y},#{z}\) \S+ [^0]\d+\s/
  end

  def unpowered(x, y, z)
    /^\(#{x},#{y},#{z}\) \S+ 0\s/
  end
end

RSpec.configure do |config|
  config.run_all_when_everything_filtered = true
  config.filter_run :focus
  config.expect_with(:rspec) {|c| c.syntax = :should}
end
