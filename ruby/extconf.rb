require 'mkmf'

find_header("hidapi.h", "../hidapi")
#find_library("hid", nil, "..")

if Config::CONFIG["target_os"] =~ /darwin/
  #find_library("hid", nil, "../mac")
  $objs = ["hidapi.o", "../mac/hid.o"]
  $libs += " -framework IOKit -framework CoreFoundation" 
end

if Config::CONFIG["target_os"] =~ /linux/
  #find_library("hid", nil, "../linux")
  $objs = ["hidapi.o", "../linux/hid-libusb.o"]
  dir_config("usb-1.0");
  dir_config("udev");
  find_library("usb-1.0", nil)
  find_library("udev", nil)
end

if Config::CONFIG["target_os"] =~ /mswin/
  #find_library("hid", nil, "../windows/Release")
  $objs = ["hidapi.o", "../windows/Release/hid.o"]
end

create_makefile('hidapi')
