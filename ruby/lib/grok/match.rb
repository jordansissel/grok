require "rubygems"
require "ffi"
require "grok"

class Grok::Match < FFI::Struct
  module CGrokMatch
    extend FFI::Library
    ffi_lib "libgrok.so"

    attach_function :grok_match_get_named_substring,
                    [:pointer, :string], :pointer
    attach_function :grok_match_walk_init, [:pointer], :void
    attach_function :grok_match_walk_next,
                    [:pointer, :pointer, :pointer, :pointer, :pointer], :int
    attach_function :grok_match_walk_end, [:pointer], :void
  end

  include CGrokMatch
  layout :grok, :pointer,
         :subject, :string,
         :start, :int,
         :end, :int

  public
  def initialize
    super

    @captures = nil
  end

  private
  def _get_captures
    @captures = Hash.new { |h, k| h[k] = Array.new }
    grok_match_walk_init(self)
    name_ptr = FFI::MemoryPointer.new(:string)
    namelen_ptr = FFI::MemoryPointer.new(:pointer)
    data_ptr = FFI::MemoryPointer.new(:string)
    datalen_ptr = FFI::MemoryPointer.new(:pointer)
    while grok_match_walk_next(self, name_ptr, namelen_ptr, data_ptr, datalen_ptr) == Grok::GROK_OK
      namelen = namelen_ptr.read_int
      name = name_ptr.get_pointer(0).read_string.dup.slice(0, namelen)
      datalen = datalen_ptr.read_int
      data = data_ptr.get_pointer(0).read_string.dup.slice(0, datalen)
      @captures[name] << data
    end
    grok_match_walk_end(self)
  end

  public
  def captures
    _get_captures if @captures.nil?
    return @captures
  end

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

  public
  def each_capture
    _get_captures if @captures.nil?
    @captures.each { |k, v| yield([k, v]) }
  end
end # Grok::Match
