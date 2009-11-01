require 'rubygems'
require 'Grok'
require 'test/unit'

class GrokPatternCapturingTests < Test::Unit::TestCase
  def setup
    @grok = Grok.new
  end

  def test_capture_methods
    @grok.add_pattern("foo", ".*")
    @grok.compile("%{foo}")
    match = @grok.match("hello world")
    assert_respond_to(match, :captures)
    assert_respond_to(match, :start)
    assert_respond_to(match, :end)
    assert_respond_to(match, :subject)
    assert_respond_to(match, :each_capture)
  end

  def test_basic_capture
    @grok.add_pattern("foo", ".*")
    @grok.compile("%{foo}")
    input = "hello world"
    match = @grok.match(input)
    assert_equal("(?<0000>.*)", @grok.expanded_pattern)
    assert_kind_of(GrokMatch, match)
    assert_kind_of(Hash, match.captures)
    assert_equal(match.captures.length, 1)
    assert_kind_of(Array, match.captures["foo"])
    assert_equal(1, match.captures["foo"].length)
    assert_kind_of(String, match.captures["foo"][0])
    assert_equal(input, match.captures["foo"][0])

    assert_kind_of(Fixnum, match.start)
    assert_kind_of(Fixnum, match.end)
    assert_kind_of(String, match.subject)
    assert_equal(0, match.start,
                 "Match of /.*/, start should equal 0")
    assert_equal(input.length, match.end,
                 "Match of /.*/, end should equal input string length")
    assert_equal(input, match.subject)
  end

  def test_multiple_captures_with_same_name
    @grok.add_pattern("foo", "\\w+")
    @grok.compile("%{foo} %{foo}")
    match = @grok.match("hello world")
    assert_not_equal(nil, match)
    assert_equal(1, match.captures.length)
    assert_equal(2, match.captures["foo"].length)
    assert_equal("hello", match.captures["foo"][0])
    assert_equal("world", match.captures["foo"][1])
  end

  def test_multiple_captures
    @grok.add_pattern("foo", "\\w+")
    @grok.add_pattern("bar", "\\w+")
    @grok.compile("%{foo} %{bar}")
    match = @grok.match("hello world")
    assert_not_equal(nil, match)
    assert_equal(2, match.captures.length)
    assert_equal(1, match.captures["foo"].length)
    assert_equal(1, match.captures["bar"].length)
    assert_equal("hello", match.captures["foo"][0])
    assert_equal("world", match.captures["bar"][0])
  end

  def test_nested_captures
    @grok.add_pattern("foo", "\\w+ %{bar}")
    @grok.add_pattern("bar", "\\w+")
    @grok.compile("%{foo}")
    match = @grok.match("hello world")
    assert_not_equal(nil, match)
    assert_equal(2, match.captures.length)
    assert_equal(1, match.captures["foo"].length)
    assert_equal(1, match.captures["bar"].length)
    assert_equal("hello world", match.captures["foo"][0])
    assert_equal("world", match.captures["bar"][0])
  end

  def test_nesting_recursion
    @grok.add_pattern("foo", "%{foo}")
    assert_raises(ArgumentError) do
      @grok.compile("%{foo}")
    end
  end
end
