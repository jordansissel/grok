PACKAGE=grok
PREFIX=/usr/local
CFLAGS+=-O2
#CFLAGS+=-g

PLATFORM=$(shell (uname -o || uname -s) 2> /dev/null)
FLEX?=flex

FORCE?=0

# On FreeBSD, you may want to set GPERF=/usr/local/bin/gperf since
# the base system gperf is too old.
ifeq ($(PLATFORM), FreeBSD)
GPERF?=/usr/local/bin/gperf
else
GPERF?=/usr/bin/gperf
endif

# For linux, we need libdl for dlopen()
# On FreeBSD, comment this line out.
ifeq ($(PLATFORM), GNU/Linux)
LDFLAGS+=-ldl
endif

# You probably don't need to make changes below
CFLAGS+=-pipe -fPIC
LDFLAGS+=-lpcre -levent -rdynamic -ltokyocabinet

# Sane includes
CFLAGS+=-I/usr/local/include
LDFLAGS+=-L/usr/local/lib

# Uncomment to totally disable logging features
#CFLAGS+=-DNOLOGGING

EXTRA_CFLAGS?=
EXTRA_LDFLAGS?=
CFLAGS+=$(EXTRA_CFLAGS)
LDFLAGS+=$(EXTRA_LDFLAGS)

### End of user-servicable configuration

CLEANGEN=filters.c grok_matchconf_macro.c *.yy.c *.tab.c *.tab.h
CLEANOBJ=*.o *_xdr.[ch]
CLEANBIN=main grokre grok conftest grok_program

GROKOBJ=grok.o grokre.o grok_capture.o grok_pattern.o stringhelper.o \
        predicates.o grok_capture_xdr.o grok_match.o grok_logging.o \
        grok_program.o grok_input.o grok_matchconf.o libc_helper.o \
        grok_matchconf_macro.o filters.o
GROKPROGOBJ=grok_input.o grok_program.o grok_matchconf.o $(GROKOBJ)

.PHONY: all
all: grok libgrok.so

.PHONY: package build-package test-package update-version
package: build-package test-package 

update-version:
	sed -i -e "s/^Version: .*/Version: $$(date "+%Y%m%d")/" grok.spec

build-package: update-version
	PACKAGE=$(PACKAGE) sh package.sh

test-package:
	PKGVER=$(PACKAGE)-`date "+%Y%m%d"`; \
	tar -C /tmp -zxf $${PKGVER}.tar.gz; \
	echo "Running tests..." && $(MAKE) -C /tmp/$${PKGVER}/test test

.PHONY: clean 
clean: cleanobj cleanbin 

# reallyclean also purges generated files
# we don't clean generated files in 'clean' target
# because some systems don't have the tools to regenerate
# the data, such as FreeBSD which has the wrong flavor
# of flex (not gnu flex)
.PHONY: reallyclean
reallyclean: clean cleangen

.PHONY: cleanobj
cleanobj:
	rm -f $(CLEANOBJ)

.PHONY: cleanbin
cleanbin:
	rm -f $(CLEANBIN)

.PHONY: cleangen
cleangen:
	rm -f $(CLEANGEN)

#.PHONY: test
#test:
	#$(MAKE) -C test test

# Binary creation
grok: LDFLAGS+=-levent
grok: $(GROKOBJ) conf.tab.o conf.yy.o main.o grok_config.o
	gcc $(LDFLAGS) -g $^ -o $@

libgrok.so: 
libgrok.so: $(GROKOBJ) 
	gcc $(LDFLAGS) -fPIC -shared $^ -o $@

# File dependencies
# generated with: 
# for i in *.c; do grep '#include "' $i | fex '"2' | xargs | sed -e "s/^/$i: /"; done    
grok.c: grok.h
grok_capture.c: grok.h grok_capture.h grok_capture_xdr.h
grok_capture_xdr.c: grok_capture.h
grok_config.c: grok_input.h grok_config.h grok_matchconf.h grok_logging.h
grok_input.c: grok.h grok_program.h grok_input.h grok_matchconf.h grok_logging.h libc_helper.h
grok_logging.c: grok.h
grok_match.c: grok.h
grok_matchconf.c: grok.h grok_matchconf.h grok_matchconf_macro.h grok_logging.h libc_helper.h filters.h stringhelper.h
grok_pattern.c: grok.h grok_pattern.h
grok_program.c: grok.h grok_program.h grok_input.h grok_matchconf.h
grokre.c: grok.h predicates.h stringhelper.h
libc_helper.c: libc_helper.h
main.c: grok.h grok_program.h grok_config.h conf.tab.h
predicates.c: grok_logging.h predicates.h
stringhelper.c: stringhelper.h
filters.h: grok.h
grok.h: grok_logging.h grok_pattern.h grok_capture.h grok_match.h grokre.h
grok_capture.h: grok_capture_xdr.h
grok_config.h: grok_program.h
grok_input.h: grok_program.h
grok_match.h: grok_capture_xdr.h
grok_matchconf.h: grok.h grok_input.h grok_program.h
predicates.h: grok.h


# Output generation
grok_capture_xdr.o: grok_capture_xdr.c grok_capture_xdr.h
grok_capture_xdr.c: grok_capture.x
	[ -f $@ ] && rm $@ || true
	rpcgen -c $< -o $@
grok_capture_xdr.h: grok_capture.x
	[ -f $@ ] && rm $@ || true
	rpcgen -h $< -o $@

%.c: %.gperf
	@if $(GPERF) --version | head -1 | egrep -v '3\.[0-9]+\.[0-9]+' ; then \
		echo "We require gperf version >= 3.0.3" ; \
		exit 1; \
	fi
	$(GPERF) $< > $@

conf.tab.c conf.tab.h: conf.y
	bison -d $<

conf.yy.c: conf.lex conf.tab.h
	@if $(FLEX) --version | grep '^flex version' ; then \
		if [ "$(FORCE)" -eq 1 ] ; then \
			echo "Bad version of flex detected, but FORCE is set, trying anyway."; \
			exit 0; \
		fi; \
		echo "Fatal - cannot build"; \
		echo "You need GNU flex. You seem to have BSD flex?"; \
		strings `which flex` | grep Regents; \
		echo "If you want to try your flex, anyway, set FORCE=1"; \
		exit 1; \
	fi
	$(FLEX) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

%.1: %.pod
	pod2man -c "" -r "" $< $@


install: libgrok.so grok
	install -m 755 -o root -g root grok $(PREFIX)/bin
	install -m 644 -o root -g root libgrok.so $(PREFIX)/lib
	for header in grok.h grokre.h grok_pattern.h grok_capture.h grok_capture_xdr.h grok_match.h grok_logging.h; do \
		install -m 644 -o root -g root $$header $(PREFIX)/include; \
	done 

