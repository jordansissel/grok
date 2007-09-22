

#re = /(?<foo>\b\w+\b) .*? (?<bar>\b\w+\b)/
#re = /(?<one>\w+) \d+ (?<two>\w+)/
# Match '<word> .*? <word>
re = /(?<one>\w+) .*? (?<two>\w+)/
str = "test 1234 test foo 456 bar"
#str.scan(re).each { |x|
str.gsub(re) { |m|
  puts m
}
#m = re.match("test 1234 foo 456 bar")
