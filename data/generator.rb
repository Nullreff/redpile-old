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
    f.puts 'TICK'
  end
end

generate('100x100grid.txt', (-100..100), (-100..100), 0) do |x, y, z|
  torch = x % 10 == 0 && y % 10 == 0
  "SET #{x} #{y} #{z} #{torch ? 'TORCH UP' : 'WIRE'}"
end