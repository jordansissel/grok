require 'rubygems'
require 'Grok'
require 'test/unit'

class NumberPatternsTest < Test::Unit::TestCase
  def setup
    @grok = Grok.new
    @grok.add_patterns_from_file("../../../grok-patterns")
  end

  def test_match_number
    @grok.compile("%{NUMBER}")
    # step of a prime number near 100 so we get about 2000 iterations
    # with a somewhat random selection numbers.
    count = 0
    -100000.step(100000, 97) do |value|
      count += 1
      match = @grok.match(value.to_s)
      assert_not_equal(false, match)
      assert_equal(value.to_s, match.captures["NUMBER"][0])
    end

    value = "hello 12345 world"
    match = @grok.match(value)
    assert_not_equal(false, match)
    assert_equal("12345", match.captures["NUMBER"][0])
  end

  def test_no_match_number
    @grok.compile("%{NUMBER}")
    ["foo", "", " ", ".", "hello world", "-abcd"].each do |value|
      match = @grok.match(value.to_s)
      assert_equal(false, match)
    end
  end

  def test_match_base16num
    @grok.compile("%{BASE16NUM}")
    # Ruby represents negative values in a strange way, so only
    # test positive numbers for now.
    # I don't think anyone uses negative values in hex anyway...
    0.upto(1000) do |value|
      [("%x" % value), ("0x%08x" % value), ("%016x" % value)].each do |hexstr|
        match = @grok.match(hexstr)
        assert_not_equal(false, match)
        assert_equal(hexstr, match.captures["BASE16NUM"][0])
      end
    end
  end
end
