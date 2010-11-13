require "Grok"
require "grok/pile"

# extend Grok to add simpler access to the discover feature.
class Grok
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
end
