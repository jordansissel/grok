#!/usr/bin/env ruby
#
# Test what we can do in ruby.

#$grok_re = /(?<substr>%\((?<fullname>(?<patname>:(?:_)?[A-Za-z0-9]+)(?:_(?<subname>[A-Za-z0-9]+))?)\))/
$grok_re = /%(?<fullname>(?<patname>[A-Za-z0-9]+)(?<subname>[A-Za-z0-9]+)?)%/

def gen_pathash(fullname)
  return fullname.downcase.gsub(/[^a-z0-9]/, ".")
end

class GrokPatterns
  @@default_patterns = {
    'NUMBER' => /(?:[+-]?(?:(?:[0-9]+(?:\.[0-9]*)?)|(?:\.[0-9]+)))/,
    'WORD' => /\w+/,
  }

  def default_patterns
    @@default_patterns
  end
end

class GrokExpression
  def initialize(expr, patterns = {})
    @patterns = GrokPatterns.new.default_patterns
    @patterns.merge(patterns)
    @expr = expr
    generate_re(expr)
  end

  def re
    @re
  end

  def generate_re(string)
    count = 0
    patcounts = {}
    patcounts.default = 0

    @patmap = {}

    while count < 20
      m = $grok_re.match(string)
      break if !m

      patname = m["patname"]
      if @patterns.member?(patname)
        re = @patterns[patname].to_s
        fullname = m["fullname"]
        fullname = "#{fullname}_#{patcounts[fullname]}" if patcounts[fullname] > 0
        named_capture = gen_pathash(fullname) 
        @patmap[named_capture] = fullname 
        string[m.begin(0) ... m.end(0)] = "(?<#{named_capture}>#{re})"
      end
      patcounts[m["fullname"]] += 1
      count += 1
    end

    @re = Regexp.new(string)
  end

  def match(string)
    m = @re.match(string)
    return nil if !m

    groups = {}

    @patmap.each_key { ||
  end
end

x = GrokExpression.new("%NUMBER%")
puts x.re
puts x.patmap

#re_groups = {}

#m = re.match("1.23 4444 foobar")
#patmap.each_key { |k| 
  #re_groups[patmap[k]] = m[k]
#}
