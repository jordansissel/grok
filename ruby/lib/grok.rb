require "rubygems"
require "ffi"

class Grok < FFI::Struct
  attr_accessor :pattern
  attr_accessor :expanded_pattern
  
  PATTERN_RE = \
    /%{    # match '%{' not prefixed with '\'
     (?<name>     # match the pattern name
       (?<pattern>[A-z0-9]+)
       (?::(?<subname>[A-z0-9_:]+))?
     )}/x

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
    #super(grok_new)
    @patterns = {}
  end # def initialize

  public
  def add_pattern(name, pattern)
    @patterns[name] = pattern
    return nil
  end

  public
  def add_patterns_from_file(path)
    file = File.new(path, "r")
    file.each do |line|
      next if line =~ /^\s*#/
      name, pattern = line.gsub(/^\s*/, "").split(/\s+/, 2)
      add_pattern(name, pattern)
    end
    return nil
  end # def add_patterns_from_file

  public
  def compile(pattern)
    iterations_left = 100
    original_pattern = pattern
    loop do
      if iterations_left == 0
        raise "Deep recursionon pattern compilation of #{original_pattern.inspect}"
      end
      iterations_left -= 1
      m = PATTERN_RE.match(pattern)
      break if !m

      if @patterns.include?(m["pattern"])
        pattern.gsub!(m[0], @patterns[m["pattern"]])
      end
      p m => pattern
      sleep 1
    end
    puts "Done compiling"
    p pattern
  end

  public
  def match(text)
    match = Grok::Match.new
    text_c = FFI::MemoryPointer.from_string(text)
    rc = grok_execn(self, text_c, text.size, match)
    case rc
    when GROK_OK
      # Give the Grok::Match object a reference to the 'text_c'
      # object which is also Grok::Match#subject string;
      # this will prevent Ruby from garbage collecting it until
      # the match object is garbage collectd.
      #
      # If we don't do this, then 'text_c' will fall out of
      # scope at the end of this function and become a candidate
      # for garbage collection, causing Grok::Match#subject to become
      # corrupt and any captures to point to those corrupt portions.
      # http://code.google.com/p/logstash/issues/detail?id=47
      match.subject_memorypointer = text_c

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
