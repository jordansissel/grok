#!/usr/bin/env ruby

require 'rubygems'
require 'Grok'
#require 'ruby-prof'
require 'pp'

#RubyProf.start  

iterations = 100000
pattern = "[A-z0-9_-]*\\[[0-9]+\\]"

grok = Grok.new
grok.add_patterns_from_file("../../patterns/base")
grok.compile("%{COMBINEDAPACHELOG}")

#rubyre = Regexp.new("(?<foo>#{pattern})")
#rubyre = Regexp.new(pattern)

matches = { :grok => 0, :rubyre => 0 }
failures = { :grok => 0, :rubyre => 0 }
def time(iterations, &block)
  start = Time.now
  file = File.open("/b/logs/access")
  data = (1 .. iterations).collect { file.readline() }
  data.each do |line|
    block.call(line)
  end
  return Time.now - start
end

groktime = time(iterations) do |line|
  m = grok.match(line)
  if m
    matches[:grok] += 1
    m.captures["FOO"]
  else
    #puts line
    failures[:grok] +=1
  end
end

#rubyretime = time(iterations) do |line|
  #m = rubyre.match(line)
  #if m 
    #matches[:rubyre] += 1
    #m["foo"]
  #end
#end

puts "Grok: #{matches[:grok] / groktime}"
puts matches.inspect
puts failures.inspect
#puts "rubyre: #{rubyretime}"
#puts matches.inspect
#result = RubyProf.stop
#printer = RubyProf::FlatPrinter.new(result)
#printer.print(STDOUT, 0)
