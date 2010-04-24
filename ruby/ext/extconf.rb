require "mkmf"
find_header("tcutil.h", "/usr/local/include")
find_header("pcre.h", "/usr/local/include")
find_header("grok.h", "/usr/local/include", "../../")
find_library("grok", "grok_init", "../", "../../", "/usr/local/lib")
create_makefile("Grok")
