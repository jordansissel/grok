
.PHONY: test
test:
	$(MAKE) -C ../../ libgrok.so
	[ ! -f "../Makefile" ] && (cd ../; ruby extconf.rb)
	$(MAKE) -C ../ Grok.so
	LD_LIBRARY_PATH="$$PWD/../../" RUBYLIB="$$PWD/../" ruby alltests.rb
