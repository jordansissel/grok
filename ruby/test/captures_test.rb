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
                 "Match of /.*/, start should equal 0"))
    assert_equal(input.length, match.end,
                 "Match of /.*/, end should equal input string length"))
    assert_equal(input, match.subject)
  end
end
