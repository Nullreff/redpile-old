module Helpers
  class Redpile
    def initialize(process)
      @process = process
    end

    def run(*commands)
      commands.each {|cmd| @process.puts cmd}
      @process.close_write
      result = @process.read
      @process.close
      result
    end
  end

  def redpile(opts = '')
    process = IO.popen("./build/redpile #{opts} 2>&1", 'r+')
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
  config.order = 'random'
end
