require "mkmf"
find_header("tcutil.h", "/usr/local/include")
find_header("pcre.h", "/usr/local/include")
find_header("grok.h", "..")
find_library("grok", "grok_init", "..")

create_makefile("Grok")
