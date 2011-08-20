require 'test/unit'
$: << File.join(File.dirname(__FILE__), "..", "..", "..", "lib")

Dir["#{File.dirname(__FILE__)}/*/**/*.rb"].each do |file|
  puts "Loading tests: #{file}"
  require file
end

