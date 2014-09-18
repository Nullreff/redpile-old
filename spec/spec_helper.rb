module Helpers
  DIRECTIONS = %w(NORTH SOUTH EAST WEST UP DOWN)
  REDPILE_CONF = 'conf/redstone.lua'
  REDPILE_CMD = './build/src/redpile'
  VALGRIND_CMD = 'valgrind -q --leak-check=full --show-reachable=yes '

  class Redpile
    def initialize(process, valgrind, result)
      @process = process
      @valgrind = valgrind
      @result = result
    end

    def run(*commands)
      commands.each {|cmd| @process.puts cmd}
      @process.close_write
      result = @process.read
      @process.close
      if @valgrind && result =~ /(^==\d+==[^\n]+\n)+/m
        fail $~.to_s
      elsif @result != $?.to_i
        fail "#{result}\nExited with status code #{$?.to_i}"
      end
      result
    end
  end

  def redpile(args = nil)
    if args.is_a?(String)
      @opts = args
    elsif args.is_a?(Hash)
      @opts = args[:opts]
      @config = args[:config]
      @result = args[:result]
    end

    @opts ||= ''
    @config ||= REDPILE_CONF
    @result ||= 0
    @command ||= ENV['VALGRIND'] ? VALGRIND_CMD + REDPILE_CMD : REDPILE_CMD

    cmd = "#{@command} #{@opts} #{@config} 2>&1"
    process = IO.popen(cmd, 'r+')
    Redpile.new(process, ENV['VALGRIND'], @result)
  end
  
  def run(*commands)
    redpile.run(commands)
  end

  def powered(x, y, z)
    /^\(#{x},#{y},#{z}\) \S+ power:[^0]\d+\s/
  end

  def unpowered(x, y, z)
    /^\(#{x},#{y},#{z}\) \S+ power:0\s/
  end

  def contains_node?(result, x, y, z, type, fields = {})
    if fields.empty?
       result.should =~ /^\(#{x},#{y},#{z}\) #{type}.*$/
       return
    end
    result.should =~ /^(\(#{x},#{y},#{z}\) #{type} .*)$/
    found_fields = result[/^\(#{x},#{y},#{z}\) #{type} .*$/]
    fields.each{|key,value| found_fields.should =~ /#{key}:#{value}/}
  end
end

RSpec.configure do |config|
  config.run_all_when_everything_filtered = true
  config.filter_run :focus
  config.expect_with(:rspec) {|c| c.syntax = :should}
end
