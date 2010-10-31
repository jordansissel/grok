require "mkmf"

# TODO(sissel): I don't think we need these two headers.
#if !find_header("tcutil.h", "/usr/local/include")
#if !find_header("pcre.h", "/usr/local/include")

if !find_header("grok.h", "/usr/local/include", "../../")
  raise "Could not find grok.h"
end

if !find_library("grok", "grok_init", "../", "../../", "/usr/local/lib")
  raise "Could not find libgrok, is it installed?"
end

create_makefile("Grok")

#with_cflags("-g") do
  #with_ldflags("-g") do
    #create_makefile("Grok")
  #end
#end
