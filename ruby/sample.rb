$: << File.join(File.dirname(__FILE__), "lib")
require "grok-pure"

patterns = {}

matches = [
  #"%{SYSLOGBASE} Accepted %{NOTSPACE:method} for %{DATA:user} from %{IPORHOST:client} port %{INT:port}",
  #"%{SYSLOGBASE} Did not receive identification string from %{IPORHOST:client}",
  #"%{SYSLOGBASE} error: PAM: authentication error for %{DATA:user} from %{IPORHOST:client}",
  #"%{QUOTEDSTRING}"
  "%{COMBINEDAPACHELOG}",
  #"%{UNINDEXED}hello (?=%{GREEDYDATA})%{WORD}"
  
  #"( *%{DATA:key}:%{NOTSPACE:value})+"
]

pile = Grok::Pile.new
pile.add_patterns_from_file("patterns/pure-ruby/base")
matches.collect do |m|
  pile.compile(m)
end

bytes = 0
time_start = Time.now.to_f
$stdin.each do |line|
  grok, m = pile.match(line)
  if m
    m.each_capture do |key, value|
      p key => value
    end

    #bytes += line.length
    break
  end
end

#time_end = Time.now.to_f
#puts "parse rate: #{ (bytes / 1024) / (time_end - time_start) }"
