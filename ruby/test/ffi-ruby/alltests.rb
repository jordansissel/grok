require 'test/unit'
$: << "../lib"

Dir["#{File.dirname(__FILE__)}/*/**/*.rb"].each do |file|
  puts "Loading tests: #{file}"
  require file
end

