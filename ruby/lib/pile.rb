require "grok"

# A grok pile is an easy way to have multiple patterns together so that
# you can try to match against each one.
class Grok::Pile
  def initialize
    @groks = []
    @patterns = {}
    @pattern_files = []
  end # def initialize

  def add_pattern(name, string)
    @patterns[name] = string
  end # def add_pattern

  def add_patterns_from_file(path)
    if !File.exists?(path)
      raise "File does not exist: #{path}"
    end
    @pattern_files << path
  end # def add_patterns_from_file

  def compile(pattern)
    grok = Grok.new
    @patterns.each do |name, value|
      grok.add_patterne(name, value)
    end
    @pattern_files.each do |path|
      grok.add_patterns_from_file(path)
    end
    grok.compile(pattern)
    @groks << grok
  end # def compile

  def match(string)
    @groks.each do |grok|
      match = grok.match(string)
      if match
        return [grok, match]
      end
    end
  end # def match
end # class Grok::Pile
