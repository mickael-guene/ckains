# Common compiler configuration
CC = gcc
CCLD = $(CC)
CFLAGS = -O2 -Wall
LDFLAGS =
LIBS =

# Install prefix
PREFIX = /usr/local

# One of debug, release, static
MODE = release

# Per mode flags
CFLAGS_debug=-g -O0
LDFLAGS_debug=-g -O0
CFLAGS_static=
LDFLAGS_static=--static

# Sources definition
VPATH = src
OBJS = main.o options.o core.o binding.o log.o

# Targets
all: ckains

ckains: $(OBJS)
	$(CCLD) -o $@ $(OBJS) $(CFLAGS) $(CFLAGS_$(MODE)) $(LDFLAGS) $(LDFLAGS_$(MODE)) $(LIBS) $(LIBS_$(MODE))

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
