#!/usr/bin/env ruby

require 'hid'
require 'optparse'

dev = HID::open(0x057e, 0x0306, nil)
print dev.get_manufacturer_string, "\n"
print dev.get_product_string, "\n"
print dev.get_serial_number_string, "\n"

#dev.write([0x12, 0x00, 0x30].pack( 'C*'))
dev.write([0x12, 0x00, 0x31].pack( 'C*'))

interval = 0
while true do
  s = "xxxxxxxx";
  dev.read s
  print s.unpack("H*"), "\n"
  break unless interval
  sleep interval
end

exit 0

if send_data
  #print send_data
  s = [send_data].pack("H16")
  dev.send_feature_report s
  exit 0
end

while true do
  s = "xxxxxxxx";
  dev.read s
  print s.unpack("H*"), "\n"
  break unless interval
  sleep interval
end

