module Helpers
  def redpile(opts = '')
    result = ''
    IO.popen("./build/redpile #{opts} 2>&1", 'r+') do |r|
      yield r if block_given?
      r.close_write
      result = r.read
      r.close_read
    end
    result
  end
end

RSpec.configure do |config|
  config.treat_symbols_as_metadata_keys_with_true_values = true
  config.run_all_when_everything_filtered = true
  config.filter_run :focus
  config.order = 'random'
end
