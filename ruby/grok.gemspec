Gem::Specification.new do |spec|
  files = ["sample.rb", "INSTALL"]
  dirs = %w{ext test lib}
  dirs.each do |dir|
    Dir["#{dir}/**/*"].each do |file|
      next if file =~ /^\./
      files << file
    end
  end

  svnrev = %x{svn info}.split("\n").grep(/Revision:/).first.split(" ").last.to_i
  spec.name = "jls-grok"
  spec.version = "0.1.#{svnrev}"

  spec.summary = "grok bindings for ruby"
  spec.description = "Grok ruby bindings - pattern match/extraction tool"
  spec.files = files
  spec.require_paths << "ext"
  spec.extensions = ["ext/extconf.rb"]

  spec.author = "Jordan Sissel"
  spec.email = "jls@semicomplete.com"
  spec.homepage = "http://code.google.com/p/semicomplete/wiki/Grok"
end

