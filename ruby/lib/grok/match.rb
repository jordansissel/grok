require "rubygems"
require "ffi"
require "grok"

class Grok::Match < FFI::Struct
  attr_accessor :subject
  attr_accessor :start
  attr_accessor :end
  attr_accessor :grok

  attr_accessor :match

  public
  def initialize
    @captures = nil
  end

  public
  def each_capture
    @captures = Hash.new { |h, k| h[k] = Array.new }
    grok_match_walk_init(self)
    name_ptr = FFI::MemoryPointer.new(:pointer)
    namelen_ptr = FFI::MemoryPointer.new(:int)
    data_ptr = FFI::MemoryPointer.new(:pointer)
    datalen_ptr = FFI::MemoryPointer.new(:int)
    while grok_match_walk_next(self, name_ptr, namelen_ptr, data_ptr, datalen_ptr) == Grok::GROK_OK
      namelen = namelen_ptr.read_int
      name = name_ptr.get_pointer(0).get_string(0, namelen)
      datalen = datalen_ptr.read_int
      data = data_ptr.get_pointer(0).get_string(0, datalen)
      yield name, data
    end
    grok_match_walk_end(self)
  end # def each_capture

  public
  def captures
    if @captures.nil?
      @captures = Hash.new { |h,k| h[k] = [] }
      each_capture do |key, val|
        @captures[key] << val
      end
    end
    return @captures
  end # def captures

  public
  def start
    return self[:start]
  end

  public
  def end
    return self[:end]
  end

  public
  def subject
    return self[:subject]
  end
end # Grok::Match
