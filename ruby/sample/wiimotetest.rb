#!/usr/bin/env ruby

require 'wiimote'

wiimote = Wiimote.new
wiimote.enable_motion_sensor

loop do
  wiimote.update
  puts "#{wiimote.xaxis} #{wiimote.yaxis} #{wiimote.zaxis}"
  sleep 0.1
end
