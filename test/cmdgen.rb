#!/usr/bin/env ruby

counter = 1
range = -20..20

range.each do |x|
  range.each do |y|
    range.each do |z|
      tick = counter % 20 == 0
      torch = counter % 30 == 0

      puts 'TICK' if tick
      puts "SET #{x} #{y} #{z} #{torch ? 'TORCH' : 'WIRE'}"
      counter += 1
    end
  end
end
