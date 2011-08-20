$: << File.join(File.dirname(__FILE__), "..", "..", "..", "lib")
require 'grok-pure'
require 'test/unit'

class HostPattternTest < Test::Unit::TestCase
  def setup
    @grok = Grok.new
    path = "#{File.dirname(__FILE__)}/../../../patterns/pure-ruby/base"
    @grok.add_patterns_from_file(path)
    @grok.compile("%{HOSTNAME}")
  end

  def test_hosts
    hosts = ["www.google.com", "foo-234.14.AAc5-2.foobar.net",
            "192-455.a.b.c.d."]
    hosts.each do |host|
      match = @grok.match(host)
      assert_not_equal(false, match, "Expected this to match: #{host}")
      assert_equal(host, match.captures["HOSTNAME"][0])
    end
  end
end
