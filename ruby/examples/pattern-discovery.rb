#!/usr/bin/env ruby
#

require "rubygems"
require "grok"
require "pp"

grok = Grok.new

# Load some default patterns that ship with grok.
# See also: 
#   http://code.google.com/p/semicomplete/source/browse/grok/patterns/base
grok.add_patterns_from_file("/usr/local/share/grok/patterns/base")

# Using the patterns we know, try to build a grok pattern that best matches 
# a string we give. Let's try Time.now.to_s, which has this format;
# => Fri Apr 16 19:15:27 -0700 2010
pattern = grok.discover("Time is #{Time.now}")

puts "Pattern: #{pattern}"
grok.compile(pattern)

# Sleep to change time.
sleep(2)
match = grok.match("Time is #{Time.now.to_s}")
pp match.captures

# When run, the output should look something like this:
# % ruby pattern-discovery.rb
# Pattern: Time is Fri %{SYSLOGDATE} %{BASE10NUM} 2010
# {"BASE10NUM"=>["-0700"],
#  "SYSLOGDATE"=>["Apr 16 19:17:38"],
#  "TIME"=>["19:17:38"],
#  "MONTH"=>["Apr"],
#  "MONTHDAY"=>["16"]}
