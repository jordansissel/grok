#!/usr/bin/env ruby
#
# Simple web application that will let you feed grok's discovery feature
# a bunch of data, and grok will show you patterns found and the results
# of that pattern as matched against the same input.

require 'rubygems'
require 'sinatra'
require 'grok'

get '/' do
  redirect "/demo/grok-discover/index"
end

get "/demo/grok-discover/index" do
  haml :index
end

post "/demo/grok-discover/grok" do
  grok = Grok.new
  grok.add_patterns_from_file("/usr/local/share/grok/patterns/base")
  @results = []
  params[:data].split("\n").each do |line|
    pattern = grok.discover(line)
    grok.compile(pattern)
    match = grok.match(line)
    puts "Got input: #{line}"
    puts " => pattern: (#{match != false}) #{pattern}"
    @results << { 
        :input => line,
        :pattern => grok.pattern.gsub(/\\Q|\\E/, ""),
        :full_pattern => grok.expanded_pattern,
        :match => (match and match.captures or false),
    }
  end
  haml :grok
end

get "/demo/grok-discover/style.css" do
  sass :style
end

__END__
@@ style
h1
  color: red
.original
.regexp
  display: block
  border: 1px solid grey
  padding: 1em

.results
  width: 80%
  margin-left: auto
  th
    text-align: left
  td
    border-top: 1px solid black
@@ layout
%html
  %head
    %title Grok Web
    %link{:rel => "stylesheet", :href => "/demo/grok-discover/style.css"}
  %body
    =yield

@@ index
#header
  %h1 Grok Web
#content
  Paste some log data below. I'll do my best to have grok generate a pattern for you.

  %p
    Learn more about grok here:
    %a{:href => "http://code.google.com/p/semicomplete/wiki/Grok"} Grok

  %p
    This is running off of my cable modem for now, so if it's sluggish, that's
    why. Be gentle.
  %form{:action => "/demo/grok-discover/grok", :method => "post"}
    %textarea{:name => "data", :rows => 10, :cols => 80}
    %br
    %input{:type => "submit", :value=>"submit"}

@@ grok
#header
  %h1 Grok Results
  %h3
    %a{:href => "/demo/grok-discover/index"} Try more?
#content
  %p
    Below is grok's analysis of the data you provided. Each line is analyzed
    separately. It uses grok's standard library of known patterns to give you a
    pattern that grok can use to match more logs like the lines you provided.
  %p
    The results may not be perfect, but it gives you a head start on coming up with
    log patterns for 
    %a{:href => "http://code.google.com/p/semicomplete/wiki/Grok"} grok 
    and 
    %a{:href => "http://code.google.com/p/logstash/"} logstash
  %ol
    - @results.each do |result|
      %li
        %p.original
          %b Original:
          %br= result[:input]
        %p 
          %b Pattern:
          %br
          %span.pattern= result[:pattern]
        %p
          %b 
            Generated Regular Expression
          %small
            %i You could have written this by hand, be glad you didn't have to.
          %code.regexp= result[:full_pattern].gsub("<", "&lt;")
        %p
          If you wanted to test this, you can paste the above expression into
          pcretest(1) and it should match your input. 
        %p
          %b Capture Results
          %table.results
            %tr
              %th Name
              %th Value
            - result[:match].each do |key,val|
              - val.each do |v|
                %tr
                  %td= key
                  %td= v
