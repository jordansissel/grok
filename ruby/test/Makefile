
.PHONY: test
test:
	$(MAKE) -C ../../ libgrok.so
	[ -f "../Makefile" ] || (cd ../ext; ruby extconf.rb)
	$(MAKE) -C ../ext Grok.so
	LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$PWD/../../" RUBYLIB="$$PWD/../ext" ruby alltests.rb
