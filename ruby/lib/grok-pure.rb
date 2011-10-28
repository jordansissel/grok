require "rubygems"
require "logger"
require "cabin"

# TODO(sissel): Check if 'grok' c-ext has been loaded and abort?
class Grok
  attr_accessor :pattern
  attr_accessor :expanded_pattern
  attr_accessor :logger
  
  PATTERN_RE = \
    /%\{    # match '%{' not prefixed with '\'
       (?<name>     # match the pattern name
         (?<pattern>[A-z0-9]+)
         (?::(?<subname>[A-z0-9_:]+))?
       )
       (?:=(?<definition>
         (?:
           (?:[^{}\\]+|\\.+)+
           |
           (?<curly>\{(?:(?>[^{}]+|(?>\\[{}])+)|(\g<curly>))*\})+
         )+
       ))?
       [^}]*
     \}/x

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
    @patterns = {}
    @logger = Cabin::Channel.new
    @logger.subscribe(Logger.new(STDOUT))
    @logger.level = :warn

    # TODO(sissel): Throw exception if we aren't using Ruby 1.9.2 or newer.
  end # def initialize

  public
  def add_pattern(name, pattern)
    @logger.info("Adding pattern", name => pattern)
    @patterns[name] = pattern
    return nil
  end

  public
  def add_patterns_from_file(path)
    file = File.new(path, "r")
    file.each do |line|
      next if line =~ /^\s*#/
      name, pattern = line.gsub(/^\s*/, "").split(/\s+/, 2)
      next if pattern.nil?
      add_pattern(name, pattern.chomp)
    end
    return nil
  end # def add_patterns_from_file

  public
  def compile(pattern)
    @capture_map = {}

    iterations_left = 100
    @pattern = pattern
    @expanded_pattern = pattern
    index = 0

    # Replace any instances of '%{FOO}' with that pattern.
    loop do
      if iterations_left == 0
        raise "Deep recursion pattern compilation of #{pattern.inspect} - expanded: #{@expanded_pattern.inspect}"
      end
      iterations_left -= 1
      m = PATTERN_RE.match(@expanded_pattern)
      break if !m

      if m["definition"]
        add_pattern(m["pattern"], m["definition"])
      end

      if @patterns.include?(m["pattern"])
        # create a named capture index that we can push later as the named
        # pattern. We do this because ruby regexp can't capture something
        # by the same name twice.
        p = @patterns[m["pattern"]]

        capture = "a#{index}" # named captures have to start with letters?
        #capture = "%04d" % "#{index}" # named captures have to start with letters?
        replacement_pattern = "(?<#{capture}>#{p})"
        #p(:input => m[0], :pattern => replacement_pattern)
        @capture_map[capture] = m["name"]
        @expanded_pattern.sub!(m[0], replacement_pattern)
        index += 1
      end
    end

    @regexp = Regexp.new(@expanded_pattern)
    @logger.debug("Grok compiled OK", :pattern => pattern,
                  :expanded_pattern => @expanded_pattern)
  end # def compile

  public
  def match(text)
    match = @regexp.match(text)

    if match
      grokmatch = Grok::Match.new
      grokmatch.subject = text
      grokmatch.start, grokmatch.end = match.offset(0)
      grokmatch.grok = self
      grokmatch.match = match
      @logger.debug("Regexp match object", :names => match.names, :captures => match.captures)
      return grokmatch
    else
      return false
    end
  end # def match

  public
  def discover(input)
    init_discover if @discover == nil

    return @discover.discover(input)
  end # def discover

  private
  def init_discover
    @discover = GrokDiscover.new(self)
    @discover.logmask = logmask
  end # def init_discover

  public
  def capture_name(id)
    return @capture_map[id]
  end # def capture_name
end # Grok

require "grok/pure/match"
require "grok/pure/pile"
