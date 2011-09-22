
PLATFORM=$(shell (uname -o || uname -s) 2> /dev/null)

ifeq ($(PLATFORM), Darwin)
LIBSUFFIX=dylib
else
LIBSUFFIX=so
endif

../../libgrok.$(LIBSUFFIX):
	$(MAKE) -C ../../ libgrok.$(LIBSUFFIX)

.PHONY: test
test-pure:
	JRUBY_OPTS=--1.9 LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../lib" ruby pure-ruby/alltests.rb


test-ffi: ../../libgrok.$(LIBSUFFIX)
	JRUBY_OPTS=--1.9 LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../lib" ruby ffi-ruby/alltests.rb
