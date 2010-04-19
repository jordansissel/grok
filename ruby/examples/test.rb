#!/usr/bin/env ruby
#

require "rubygems"
require "grok"
require "pp"

grok = Grok.new

# Load some default patterns that ship with grok.
# See also: 
#   http://code.google.com/p/semicomplete/source/browse/grok/patterns/base
grok.add_patterns_from_file("../..//patterns/base")

# Using the patterns we know, try to build a grok pattern that best matches 
# a string we give. Let's try Time.now.to_s, which has this format;
# => Fri Apr 16 19:15:27 -0700 2010
input = "2010-04-18T15:06:02Z"
pattern = "%{TIMESTAMP_ISO8601}"
grok.compile(pattern)
grok.compile(pattern)
puts "Input: #{input}"
puts "Pattern: #{pattern}"
puts "Full: #{grok.expanded_pattern}"

match = grok.match(input)
if match
  puts "Resulting capture:"
  pp match.captures
end
