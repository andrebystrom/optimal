CC=gcc
CFLAGS=-c -g -std=c99 -Wall -Wextra -fsanitize=address,undefined,leak
LDFLAGS=-fsanitize=address,undefined,leak

SRCDIR=$(PWD)/src
BUILDDIR=$(PWD)/build

ifdef RELEASE
	CFLAGS-=-fsanitize=address,undefined,leak
	LDFLAGS-=-fsanitize=address,undefined,leak
endif

$(BUILDDIR)/test: $(BUILDDIR)/test.o $(BUILDDIR)/optimal.o
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/test.o: $(SRCDIR)/test.c $(SRCDIR)/optimal.h
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/optimal.o: $(SRCDIR)/optimal.c $(SRCDIR)/optimal.h
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILDDIR)