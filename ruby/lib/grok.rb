require "Grok"

# extend Grok to add simpler access to the discover feature.
class Grok
  def discover(input)
    init_discover if @discover == nil

    return @discover.discover(input)
  end

  def init_discover
    @discover = GrokDiscover.new(self)
  end
end
