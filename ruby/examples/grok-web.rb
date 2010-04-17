#!/usr/bin/env ruby
#
# Simple web application that will let you feed grok's discovery feature
# a bunch of data, and grok will show you patterns found and the results
# of that pattern as matched against the same input.

require 'rubygems'
require 'sinatra'
require 'grok'

get '/' do
  haml :index
end

post '/grok' do
  grok = Grok.new
  grok.add_patterns_from_file("/usr/local/share/grok/patterns/base")
  @results = []
  params[:data].split("\n").each do |line|
    pattern = grok.discover(line)
    puts :OK
    puts pattern
    grok.compile(pattern)
    match = grok.match(line)
    @results << { 
        :input => line,
        :pattern => grok.pattern.gsub(/\\Q|\\E/, ""),
        :full_pattern => grok.expanded_pattern,
        :match => (match and match.captures or false),
    }
  end
  haml :grok
end

get "/style.css" do
  sass :style
end

__END__
@@ style
h1
  color: red
.original
.pattern
.regexp
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
    %link{:rel => "stylesheet", :href => "/style.css"}
  %body
    =yield

@@ index
#header
  %h1 Grok Web
#content
  Paste some log data below. I'll do my best to have grok generate a pattern for you.
  %form{:action => "/grok", :method => "post"}
    %textarea{:name => "data", :rows => 10, :cols => 80}
    %br
    %input{:type => "submit", :value=>"submit"}

@@ grok
#header
  %h1 Grok Results
  %h3
    %a{:href => "/"} Try more?
#content
  %dl
  - @results.each do |result|
    %dt.original= result[:input]
    %dd
      %p 
        %b Pattern:
        %code.pattern= result[:pattern]
      %p
        %b Regexp: 
        %code.regexp= result[:full_pattern]
      %p
        %b Capture Results
        %table.results
          %tr
            %th Name
            %th Value
          - result[:match].each do |key,val|
            %tr
              %td= key
              %td= val
