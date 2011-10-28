Gem::Specification.new do |spec|
  files = Dir.glob("lib/**/*.rb")
  files + Dir.glob("patterns/**")
  files + Dir.glob("test/")

  #svnrev = %x{svn info}.split("\n").grep(/Revision:/).first.split(" ").last.to_i
  spec.name = "jls-grok"
  spec.version = "0.9.2"

  spec.summary = "grok bindings for ruby"
  spec.description = "Grok ruby bindings - pattern match/extraction tool"
  spec.files = files

  # TODO(sissel): ffi is now optional, get rid of it?
  #spec.add_dependency("ffi", "> 0.6.3")
  spec.require_paths << "lib" 

  # Cabin for logging.
  spec.add_dependency("cabin")

  spec.authors = ["Jordan Sissel", "Pete Fritchman"]
  spec.email = ["jls@semicomplete.com", "petef@databits.net"]
  spec.homepage = "http://code.google.com/p/semicomplete/wiki/Grok"
end

