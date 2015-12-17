# Common compiler configuration
CC = gcc
CCLD = $(CC)
CFLAGS = -O2 -Wall
LDFLAGS =
LIBS =
STRIP = strip

# Install prefix
PREFIX = /usr/local

# One of debug, release, static
MODE = release

# Per mode flags
STRIP_release = $(STRIP)
CFLAGS_debug=-g -O0
LDFLAGS_debug=-g -O0
STRIP_debug = true
CFLAGS_static=
LDFLAGS_static=--static
STRIP_static = $(STRIP)

# Sources definition
VPATH = src
OBJS = main.o options.o core.o binding.o log.o

# Targets
all: ckains

ckains: $(OBJS)
	$(CCLD) -o $@ $(OBJS) $(CFLAGS) $(CFLAGS_$(MODE)) $(LDFLAGS) $(LDFLAGS_$(MODE)) $(LIBS) $(LIBS_$(MODE))
	$(STRIP_$(MODE)) $@

$(OBJS): %.o: %.c
	$(CC) -o $@ $< -c $(CFLAGS) $(CFLAGS_$(MODE))

clean:
	rm -f *.o && rm -f ckains

distclean: clean

install:
	install -d $(PREFIX)/bin
	install ckains $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/ckains

$(OBJS) ckains: Makefile
