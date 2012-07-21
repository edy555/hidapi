#!/usr/bin/env ruby

require 'hid'

dev = HID::open(0x16c0, 0x05df, nil)
#print dev.get_manufacturer_string, "\n"
#print dev.get_product_string, "\n"
#print dev.get_serial_number_string, "\n"

while true do
  s = "xx";
  dev.read s
  print s.unpack("S"), "\n"
  sleep 1
end

