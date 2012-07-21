#!/usr/bin/env ruby

require 'hid'
require 'optparse'

send_data = nil
interval = nil
opts = OptionParser.new
opts.on("-s DATA", String, /[0-9a-fA-F]+/){|data| send_data = data}
opts.on("-l [INTERVAL]", Integer, /[0-9]+/, "repeat reading data with INTERVAL"){|i| interval = i || 1}
opts.parse!(ARGV)

dev = HID::open(0x16c0, 0x05df, nil)
#print dev.get_manufacturer_string, "\n"
print dev.get_product_string, "\n"
#print dev.get_serial_number_string, "\n"

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

