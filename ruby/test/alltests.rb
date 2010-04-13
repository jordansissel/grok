require 'test/unit'

Dir["#{File.dirname(__FILE__)}/*/**/*.rb"].each do |file|
  puts "Loading tests: #{file}"
  require file
end

