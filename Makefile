#
# This file is part of ckains, a c clone of Kains.
#
# Copyright (C) 2015 STMicroelectronics
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA.
#

# Common compiler configuration
CC = gcc
CCLD = $(CC)
CFLAGS = -O2 -Wall
LDFLAGS =
LIBS =
STRIP = strip
GIT = git

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
VPATH = src test
OBJS = main.o options.o core.o binding.o log.o
TESTS = test001.sh test002.sh

# Versioning
GIT_VERSION := $(shell ${GIT} describe --abbrev=0)
GIT_DESCRIBE := $(shell ${GIT} describe --always --tags --long --abbrev=12 --dirty)

# Targets
all: ckains

ckains: $(OBJS)
	$(CCLD) -o $@ $(OBJS) $(CFLAGS) $(CFLAGS_$(MODE)) $(LDFLAGS) $(LDFLAGS_$(MODE)) $(LIBS) $(LIBS_$(MODE))
	$(STRIP_$(MODE)) $@

$(OBJS): %.o: %.c
	$(CC) -o $@ $< -c $(CFLAGS) $(CFLAGS_$(MODE)) -DGIT_VERSION=\"$(GIT_VERSION)\" -DGIT_DESCRIBE=\"$(GIT_DESCRIBE)\"

clean:
	rm -f *.o && rm -f ckains

distclean: clean

check: test

test: $(TESTS:%.sh=%.chk)

$(TESTS:%.sh=%.chk): %.chk: %.sh
	env CKAINS=./ckains $<

install:
	install -d $(PREFIX)/bin
	install ckains $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/ckains

deploy:
	scripts/deploy.sh

$(OBJS) ckains: Makefile

.PHONY: all test check clean distclean install uninstall deploy
