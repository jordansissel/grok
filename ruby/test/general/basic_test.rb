#require 'rubygems'
require 'Grok'
require 'test/unit'

class GrokBasicTests < Test::Unit::TestCase
  def setup
    @grok = Grok.new
  end

  def test_grok_methods
    assert_respond_to(@grok, :compile)
    assert_respond_to(@grok, :match)
    assert_respond_to(@grok, :expanded_pattern)
    assert_respond_to(@grok, :pattern)
  end

  def test_grok_compile_fails_on_invalid_expressions
    bad_regexps = ["[", "[foo", "?", "foo????", "(?-"]
    bad_regexps.each do |regexp|
      assert_raise ArgumentError do
        @grok.compile(regexp)
      end
    end
  end

  def test_grok_compile_succeeds_on_valid_expressions
    good_regexps = ["[hello]", "(test)", "(?:hello)", "(?=testing)"]
    good_regexps.each do |regexp|
      assert_nothing_raised do
        @grok.compile(regexp)
      end
    end
  end

  def test_grok_pattern_is_same_as_compile_pattern
    pattern = "Hello world"
    @grok.compile(pattern)
    assert_equal(pattern, @grok.pattern)
  end

  # TODO(sissel): Move this test to a separate test suite aimed
  # at testing grok internals
  def test_grok_expanded_pattern_works_correctly
    @grok.add_pattern("test", "hello world")
    @grok.compile("%{test}")
    assert_equal("(?<0000>hello world)", @grok.expanded_pattern)
  end

  def test_grok_load_patterns_from_file
    require 'tempfile'
    fd = Tempfile.new("grok_test_patterns.XXXXX")
    fd.puts "TEST \\d+"
    fd.close
    @grok.add_patterns_from_file(fd.path)
    @grok.compile("%{TEST}")
    assert_equal("(?<0000>\\d+)", @grok.expanded_pattern)
  end
end
