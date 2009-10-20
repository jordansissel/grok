require "mkmf"
find_header("grok.h", "..")
find_library("grok", "grok_init", "..")

create_makefile("Grok")
