#!/usr/bin/env ruby
require 'hidapi'

dev = HID::open 0x057e, 0x0306
print dev.get_manufacturer_string, "\n"
print dev.get_product_string, "\n"
print dev.get_serial_number_string, "\n"

#dev.write([0x12, 0x00, 0x30].pack( 'C*'))
dev.write([0x12, 0x00, 0x31].pack( 'C*'))

interval = 0
while true do
  #s = "xxxxxxxx"
  #dev.read s
  s = dev.read 8
  print s.unpack("H*"), "\n"
  break unless interval
  sleep interval
end
