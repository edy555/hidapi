#!/usr/bin/env ruby
require 'hidapi'

HID::enumerate.each do |d|
  puts d[:path]
  puts d[:serial_number]
  #puts d[:release_number]
  puts d[:manufacturer_string]
  puts d[:product_string]
  puts d[:vendor_id]
  puts d[:product_id]
  #puts d[:usage_page]
  #puts d[:usage]
  puts
end
