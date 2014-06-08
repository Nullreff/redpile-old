module Helpers
  REDPILE_CMD = './build/redpile'
  VALGRIND_CMD = 'valgrind -q --error-exitcode=1 --leak-check=full --show-reachable=yes'

  class Redpile
    def initialize(process)
      @process = process
    end

    def run(*commands)
      commands.each {|cmd| @process.puts cmd}
      @process.close_write
      result = @process.read
      @process.close
      raise "Memory Leak detected " unless $?.to_i.zero?
      result
    end
  end

  def redpile(opts = '')
    process = IO.popen("#{VALGRIND_CMD} #{REDPILE_CMD} #{opts} 2>&1", 'r+')
    Redpile.new(process)
  end
  
  def run(*commands)
    redpile.run(commands)
  end
end

RSpec.configure do |config|
  config.treat_symbols_as_metadata_keys_with_true_values = true
  config.run_all_when_everything_filtered = true
  config.filter_run :focus
end
