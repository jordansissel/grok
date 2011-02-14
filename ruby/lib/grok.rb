require "rubygems"
require "ffi"

class Grok < FFI::Struct
  module CGrok
    extend FFI::Library
    ffi_lib "libgrok.so"

    attach_function :grok_new, [], :pointer
    attach_function :grok_compilen, [:pointer, :string, :int], :int
    attach_function :grok_pattern_add,
                    [:pointer, :string, :int, :string, :int], :int
    attach_function :grok_patterns_import_from_file, [:pointer, :string], :int
    attach_function :grok_execn, [:pointer, :string, :int, :pointer], :int
  end

  include CGrok
  layout :pattern, :string,
         :pattern_len, :int,
         :full_pattern, :string,
         :full_pattern_len, :int,
         :__patterns, :pointer, # TCTREE*, technically
         :__re, :pointer, # pcre*
         :__pcre_capture_vector, :pointer, # int*
         :__pcre_num_captures, :int,
         :__captures_by_id, :pointer, # TCTREE*
         :__captures_by_name, :pointer, # TCTREE*
         :__captures_by_subname, :pointer, # TCTREE*
         :__captures_by_capture_number, :pointer, # TCTREE*
         :__max_capture_num, :int,
         :pcre_errptr, :string,
         :pcre_erroffset, :int,
         :pcre_errno, :int,
         :logmask, :uint,
         :logdepth, :uint,
         :errstr, :string

  GROK_OK = 0
  GROK_ERROR_FILE_NOT_ACCESSIBLE = 1
  GROK_ERROR_PATTERN_NOT_FOUND = 2
  GROK_ERROR_UNEXPECTED_READ_SIZE = 3
  GROK_ERROR_COMPILE_FAILED = 4
  GROK_ERROR_UNINITIALIZED = 5
  GROK_ERROR_PCRE_ERROR = 6
  GROK_ERROR_NOMATCH = 7

  public
  def initialize
    super(grok_new)
  end

  public
  def add_pattern(name, pattern)
    grok_pattern_add(self, name, name.length, pattern, pattern.length)
    return nil
  end

  public
  def add_patterns_from_file(path)
    ret = grok_patterns_import_from_file(self, path)
    if ret != GROK_OK
      raise ArgumentError, "Failed to add patterns from file #{path}"
    end
    return nil
  end

  public
  def pattern
    return self[:pattern]
  end

  public
  def expanded_pattern
    return self[:full_pattern]
  end

  public
  def compile(pattern)
    ret = grok_compilen(self, pattern, pattern.length)
    if ret != GROK_OK
      raise ArgumentError, "Compile failed: #{self[:errstr]})"
    end
    return ret
  end

  public
  def match(text)
    match = Grok::Match.new
    rc = grok_execn(self, text, text.size, match)
    case rc
    when GROK_OK
      return match
    when GROK_ERROR_NOMATCH
      return false
    end

    raise ValueError, "unknown return from grok_execn: #{rc}"
  end

  public
  def discover(input)
    init_discover if @discover == nil

    return @discover.discover(input)
  end

  private
  def init_discover
    @discover = GrokDiscover.new(self)
    @discover.logmask = logmask
  end
end # Grok

require "grok/match"
require "grok/pile"
