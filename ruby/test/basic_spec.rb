require 'rubygems'
require 'Grok'

def should_raise(klass, &block)
  block.should raise_error(klass)
end

describe Grok do
  it "should respond to compile" do
    grok = Grok.new
    grok.should respond_to(:compile)
  end

  it "should respond to match" do
    grok = Grok.new
    grok.should respond_to(:match)
  end

  it "should respond to expanded_pattern" do
    grok = Grok.new
    grok.should respond_to(:expanded_pattern)
  end

  it "should respond to pattern" do
    grok = Grok.new
    grok.should respond_to(:pattern)
  end

  bad_regexps = ["[", "[foo", "?", "foo????", "(?-"]
  bad_regexps.each do |regexp|
    it "should raise exception when compiling invalid expression /#{regexp}/" do
      grok = Grok.new
      should_raise(ArgumentError) { grok.compile(regexp) }
    end
  end

  good_regexps = ["[hello]", "(test)", "(?:hello)", "(?=testing)"]
  good_regexps.each do |regexp|
    it "should compile safely when compiling /#{regexp}/" do
      grok = Grok.new
      grok.compile(regexp)
    end
  end

  it "should report the correct unexpanded pattern" do
    grok = Grok.new
    pattern = "Hello world"
    grok.compile(pattern)
    grok.pattern.should == pattern
  end

  it "should show expanded pattern" do
    grok = Grok.new
    grok.add_pattern("test", "hello world")
    grok.compile("%{test}")
    grok.expanded_pattern.should == "(?<0000>hello world)"
  end
end
