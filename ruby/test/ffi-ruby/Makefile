
PLATFORM=$(shell (uname -o || uname -s) 2> /dev/null)

ifeq ($(PLATFORM), Darwin)
LIBSUFFIX=dylib
else
LIBSUFFIX=so
endif

.PHONY: test
test:
	$(MAKE) -C ../../ libgrok.$(LIBSUFFIX)
	LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../lib" ruby alltests.rb

test_jruby:
	$(MAKE) -C ../../ libgrok.$(LIBSUFFIX)
	LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../lib" jruby alltests.rb
