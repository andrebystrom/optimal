CC=gcc
CFLAGS=-c -g -std=c99 -Wall -Wextra
LDFLAGS=

SRCDIR=$(PWD)/src
BUILDDIR=$(PWD)/build

ifndef RELEASE
	CFLAGS+=-fsanitize=address,undefined,leak
	LDFLAGS+=-fsanitize=address,undefined,leak
else
	CFLAGS+=-O3
endif

all: $(BUILDDIR)/test

lib: $(BUILDDIR)/liboptimal.a

$(BUILDDIR)/test: $(BUILDDIR)/test.o $(BUILDDIR)/liboptimal.a
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/test.o: $(SRCDIR)/test.c $(SRCDIR)/optimal.h
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/liboptimal.a: $(BUILDDIR)/optimal.o
	ar rcs $@ $<

$(BUILDDIR)/optimal.o: $(SRCDIR)/optimal.c $(SRCDIR)/optimal.h
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -fpic $< -o $@

clean:
	rm -rf $(BUILDDIR)
