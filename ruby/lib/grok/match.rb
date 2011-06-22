require "rubygems"
require "ffi"
require "grok"

class Grok::Match
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

    @match.names.zip(@match.captures).each do |id, value|
      name = @grok.capture_name(id)
      #next if value == nil
      yield name, value
    end
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
