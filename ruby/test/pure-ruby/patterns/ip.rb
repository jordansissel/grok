require 'grok'
require 'test/unit'

class IPPatternsTest < Test::Unit::TestCase
  def setup
    @grok = Grok.new
    path = "#{File.dirname(__FILE__)}/../../../patterns/base"
    @grok.add_patterns_from_file(path)
  end

  def test_ips
    @grok.compile("%{IP}")
    File.open("#{File.dirname(__FILE__)}/ip.input").each do |line|
      line.chomp!
      match = @grok.match(line)
      assert_not_equal(false, match)
      assert_equal(line, match.captures["IP"][0])
    end
  end

  def test_non_ips
    @grok.compile("%{IP}")
    nonips = %w{255.255.255.256 0.1.a.33 300.1.2.3 300 400.4.3.a 1.2.3.b
                1..3.4.5 hello world}
    nonips << "hello world"
    nonips.each do |input|
      match = @grok.match(input)
      assert_equal(false, match)
    end
  end
end
