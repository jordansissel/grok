include ../Makefile

# Search these paths for dependency targets
VPATH=..:.

LDFLAGS+=-lcunit
CFLAGS+=-I. -I.. -g $(EXTRA_CFLAGS)
CLEANOBJ=*.o *.gentest.c *_xdr.[hco]
CLEANBIN=*.test

TESTS=$(wildcard *.test.c)
#TESTS=grok_pattern.test.c grok_capture.test.c grok_simple.test.c 

.PHONY: test
test: 
	@for t in $(TESTS:.c=); do \
		./runtest.sh $$t; \
	done

%.c: gentest.sh

%.test.o: %.test.c 
	./gentest.sh $< > _$<
	$(CC) $(CFLAGS) -c _$< -o $@;
	rm _$<


stringhelper.test: stringhelper.o
grok_pattern.test: $(GROKOBJ)
grok_capture.test: $(GROKOBJ)
grok_simple.test: $(GROKOBJ)
predicates.test: $(GROKOBJ)

%.test: %.test.o 
	$(CC) $(LDFLAGS) $(CFLAGS) $(^:cleanobj=) -o $@
