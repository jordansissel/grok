require "grok-pure"

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

    #p :expanded => @grok.expanded_pattern
    #p :map => @grok.capture_map
    @match.names.zip(@match.captures).each do |id, value|
      #p :match => id, :value => value
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
  def [](name)
    return captures[name]
  end # def []
end # Grok::Match
