
.PHONY: test
test:
	$(MAKE) -C ../../ libgrok.so
	LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../lib" ruby alltests.rb

test_jruby:
	$(MAKE) -C ../../ libgrok.so
	LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../lib" jruby alltests.rb
