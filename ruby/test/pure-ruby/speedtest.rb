#!/usr/bin/env ruby

require 'rubygems'
require 'grok'
#require 'ruby-prof'
require 'pp'

#RubyProf.start  

def main
  iterations = 1000000
  pattern = "[A-z0-9_-]*\\[[0-9]+\\]"

  grok = Grok.new
  grok.add_patterns_from_file("../../patterns/base")
  grok.compile("%{COMBINEDAPACHELOG}")

  #rubyre = Regexp.new("(?<foo>#{pattern})")
  #rubyre = Regexp.new(pattern)

  matches = 0
  failures = 0
  lines = File.new("/home/jls/access_log").readlines

  while lines.length < iterations
    lines += lines
  end
  lines = lines[0 .. iterations]

  def time(lines, iterations, &block)
    start = Time.now
    #file = File.open("/b/logs/access")
    #data = (1 .. iterations).collect { file.readline() }
    1.upto(iterations) do |i|
      block.call lines[i]
    end
    return Time.now - start
  end

  start = Time.now
  lines.each do |line|
    m = grok.match(line)
    if m
      matches += 1
      m.captures
    else
      #puts line
      failures += 1
    end
  end
  duration = Time.now - start

  #rubyretime = time(iterations) do |line|
    #m = rubyre.match(line)
    #if m 
      #matches[:rubyre] += 1
      #m["foo"]
    #end
  #end

  puts "Parse rate: #{iterations / duration}"
  puts matches.inspect
  puts failures.inspect
end

threads = []
1.upto(3) do |i|
  threads << Thread.new { main }
end
threads.each(&:join)
