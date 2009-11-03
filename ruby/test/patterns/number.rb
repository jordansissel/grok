#require 'rubygems'
require 'Grok'
require 'test/unit'

class NumberPatternsTest < Test::Unit::TestCase
  def setup
    @grok = Grok.new
    path = "#{File.dirname(__FILE__)}/../../../patterns/base"
    @grok.add_patterns_from_file(path)
  end

  def test_match_number
    @grok.compile("%{NUMBER}")
    # step of a prime number near 100 so we get about 2000 iterations
    #puts @grok.expanded_pattern.inspect
    -100000.step(100000, 97) do |value|
      match = @grok.match(value.to_s)
      assert_not_equal(false, match, "#{value} should not match false")
      assert_equal(value.to_s, match.captures["NUMBER"][0])
    end
  end

  def test_match_number_float
    # generate some random floating point values
    # always seed with the same random number, so the test is always the same
    srand(0)
    @grok.compile("%{NUMBER}")
    0.upto(1000) do |value|
      value = (rand * 100000 - 50000).to_s
      match = @grok.match(value)
      assert_not_equal(false, match)
      assert_equal(value, match.captures["NUMBER"][0])
    end
  end

  def test_match_number_amid_things
    @grok.compile("%{NUMBER}")
    value = "hello 12345 world"
    match = @grok.match(value)
    assert_not_equal(false, match)
    assert_equal("12345", match.captures["NUMBER"][0])

    value = "Something costs $55.4!"
    match = @grok.match(value)
    assert_not_equal(false, match)
    assert_equal("55.4", match.captures["NUMBER"][0])
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
