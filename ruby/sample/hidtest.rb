#!/usr/bin/env ruby

require 'hidapi'

devs = HID::enumerate
d = devs

while d do
  puts d.path
  puts d.serial_number
  puts d.release_number
  puts d.manufacturer_string
  puts d.product_string
  puts d.vendor_id
  puts d.product_id
  puts d.usage_page
  puts d.usage
  d = d.next
end

HID::free_enumeration(devs)
