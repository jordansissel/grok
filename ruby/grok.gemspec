Gem::Specification.new do |spec|
  files = <<-FILES
    INSTALL
    Rakefile
    examples
    examples/grok-web.rb
    examples/pattern-discovery.rb
    examples/test.rb
    grok.gemspec
    lib
    lib/grok
    lib/grok.rb
    lib/grok/match.rb
    lib/grok/pile.rb
    test
    test/Makefile
    test/alltests.rb
    test/general
    test/general/basic_test.rb
    test/general/captures_test.rb
    test/patterns
    test/patterns/day.rb
    test/patterns/host.rb
    test/patterns/ip.input
    test/patterns/ip.rb
    test/patterns/iso8601.rb
    test/patterns/month.rb
    test/patterns/number.rb
    test/patterns/path.rb
    test/patterns/prog.rb
    test/patterns/quotedstring.rb
    test/patterns/uri.rb
    test/run.sh
    test/speedtest.rb
  FILES

  files = files.gsub(/  +/, "").split("\n")

  svnrev = %x{svn info}.split("\n").grep(/Revision:/).first.split(" ").last.to_i
  spec.name = "jls-grok"
  spec.version = "0.3.#{svnrev}"

  spec.summary = "grok bindings for ruby"
  spec.description = "Grok ruby bindings - pattern match/extraction tool"
  spec.files = files

  spec.add_dependency("ffi", ">= 1.0.5")

  spec.authors = ["Jordan Sissel", "Pete Fritchman"]
  spec.email = ["jls@semicomplete.com", "petef@databits.net"]
  spec.homepage = "http://code.google.com/p/semicomplete/wiki/Grok"
end

