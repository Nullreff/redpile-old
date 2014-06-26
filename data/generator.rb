#!/usr/bin/env ruby

def generate(name, x_range, y_range, z_range)
  x_range = [x_range] unless x_range.respond_to?(:each)
  y_range = [y_range] unless y_range.respond_to?(:each)
  z_range = [z_range] unless z_range.respond_to?(:each)

  File.open(name, 'w') do |f|
    x_range.each do |x|
      y_range.each do |y|
        z_range.each do |z|
          f.puts yield(x, y, z)
        end
      end
    end
    f.puts 'STICK 2'
  end
end

generate('100x100grid.txt', (-50..50), 0, (-50..50)) do |x, y, z|
  torch = x % 10 == 0 && z % 10 == 0
  "SET #{x} #{y} #{z} #{torch ? 'TORCH UP' : 'WIRE'}"
end

generate('1000x1000grid.txt', (-500..500), 0, (-500..500)) do |x, y, z|
  torch = x % 100 == 0 && z % 100 == 0
  "SET #{x} #{y} #{z} #{torch ? 'TORCH UP' : 'WIRE'}"
end

generate('1000line.txt', (0..1000), 0, 0) do |x, y, z|
  if x == 0
    "SET #{x} #{y} #{z} TORCH UP"
  elsif x % 15 == 0
    "SET #{x} #{y} #{z} REPEATER EAST 0\nTICK"
  else
    "SET #{x} #{y} #{z} WIRE"
  end
end
