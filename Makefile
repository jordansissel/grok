PACKAGE=grok
PREFIX=/usr/local
CFLAGS+=-O2

PLATFORM=$(shell (uname -o || uname -s) 2> /dev/null)

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

CLEANGEN=filters.c grok_matchconf_macro.c
CLEANOBJ=*.o *_xdr.[ch] *.yy.c *.tab.c *.tab.h
CLEANBIN=main grokre grok conftest grok_program

GROKOBJ=grok.o grokre.o grok_capture.o grok_pattern.o stringhelper.o \
        predicates.o grok_capture_xdr.o grok_match.o grok_logging.o \
        grok_program.o grok_input.o grok_matchconf.o libc_helper.o \
        grok_matchconf_macro.o filters.o
GROKPROGOBJ=grok_input.o grok_program.o grok_matchconf.o $(GROKOBJ)

.PHONY: all
all: grok libgrok.so

.PHONY: package build-package test-package
package: build-package test-package

build-package:
	PACKAGE=$(PACKAGE) sh package.sh

test-package:
	PKGVER=$(PACKAGE)-`date "+%Y%m%d"`; \
	tar -C /tmp -xf $${PKGVER}.tar.gz; \
	echo "Running tests..." && make -C /tmp/$${PKGVER}/test test

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

.PHONY: test
test:
	make -C test test

# Binary creation
grok: LDFLAGS+=-levent
grok: $(GROKOBJ) conf.tab.o conf.yy.o main.o grok_config.o
	gcc $(LDFLAGS) -g $^ -o $@

libgrok.so: 
libgrok.so: $(GROKOBJ) 
	gcc $(LDFLAGS) -fPIC -shared -g $^ -o $@

# File dependencies
grok.h: grok_capture.h
grok_match.h: grok_capture.h
grok_capture.h: grok_capture_xdr.h
main.c: grok_capture.h
grok_input.c: grok_capture.h libc_helper.h
grok.c: grok.h grok_capture.h

# Output generation
grok_capture_xdr.o: grok_capture_xdr.c grok_capture_xdr.h
grok_capture_xdr.c: grok_capture.x
	[ -f $@ ] && rm $@ || true
	rpcgen -c $< -o $@
grok_capture_xdr.h: grok_capture.x
	[ -f $@ ] && rm $@ || true
	rpcgen -h $< -o $@

%.c: %.gperf
	if $(GPERF) --version | egrep -v '3\.[0-9]+\.[0-9]+' ; then \
		echo "We require gperf version >= 3.0.3" ; \
		exit 1; \
	fi
	$(GPERF) $< > $@

conf.tab.c conf.tab.h: conf.y
	bison -d $<

conf.yy.c: conf.lex conf.tab.h
	flex -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

install: libgrok.so grok
	install -m 755 -o root -g root grok $(PREFIX)/bin
	install -m 644 -o root -g root libgrok.so $(PREFIX)/lib
	for header in grok.h grokre.h grok_pattern.h grok_capture.h grok_capture_xdr.h grok_match.h grok_logging.h; do \
		install -m 644 -o root -g root $$header $(PREFIX)/include; \
	done 

