#!/usr/bin/env ruby
require 'rubygems'
require 'eventmachine'
require 'wiimote'

class EM::Wiimote
  def onstart(&blk); @onstart = blk; end
  def onupdate(&blk); @onupdate = blk; end

  def self.start(&blk)
    w = self.new
    blk.call w
    dev = ::Wiimote.new
    EM.defer do 
      w.process dev
    end
  end

  def process(dev)
    @onstart.call(dev) if @onstart
    loop do
      dev.update
      @onupdate.call(dev) if @onupdate
    end
  end
end

EM.run do
  # trap("TERM") { EM.stop }
  # trap("INT")  { EM.stop }

  EM::Wiimote.start do |w|
    w.onstart {|dev|
      dev.enable_motion_sensor
    }
    w.onupdate {|dev|
      puts "#{dev.xaxis} #{dev.yaxis} #{dev.zaxis}"
    }
  end
end
