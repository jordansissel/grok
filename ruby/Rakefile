task :default => [:package]

task :package do
  system("svn up")
  system("gem build eventmachine-tail.gemspec")
end

task :publish do
  latest_gem = %x{ls -t eventmachine-tail*.gem}.split("\n").first
  system("gem push #{latest_gem}")
end
