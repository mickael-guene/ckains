#!/usr/bin/env bash
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

set -eu
set -o pipefail
set -x

# Clean build
make distclean && make MODE=static && make install PREFIX=$PWD/devimage

# Build x86_64
[ -d rootfs-x86_64 ] && chmod -R +rwX rootfs-x86_64
rm -rf rootfs-x86_64
mkdir rootfs-x86_64
wget -O rootfs-x86_64.tar.gz http://cdimage.ubuntu.com/ubuntu-core/releases/12.04.5/release/ubuntu-core-12.04.4-core-amd64.tar.gz
(cd rootfs-x86_64 && tar xzf ../rootfs-x86_64.tar.gz 2>/dev/null || true)
devimage/bin/ckains -S rootfs-x86_64 -- sh -c 'apt-get update && apt-get install -y make gcc'
devimage/bin/ckains -R rootfs-x86_64 -b $PWD -w $PWD -- sh -c 'make distclean && make all MODE=static && make install PREFIX=$PWD/deploy/`uname -m`'
mkdir -p deploy/bin && cp deploy/x86_64/bin/ckains deploy/bin/ckains.x86_64

# Build i686
[ -d rootfs-i686 ] && chmod -R +rwX rootfs-i686
rm -rf rootfs-i686
mkdir rootfs-i686
wget -O rootfs-i686.tar.gz http://cdimage.ubuntu.com/ubuntu-core/releases/12.04.5/release/ubuntu-core-12.04.4-core-i386.tar.gz
(cd rootfs-i686 && tar xzf ../rootfs-i686.tar.gz 2>/dev/null || true)
devimage/bin/ckains --32 -S rootfs-i686 -- sh -c 'apt-get update && apt-get install -y make gcc'
devimage/bin/ckains --32 -R rootfs-i686 -b $PWD -w $PWD -- sh -c 'make distclean && make all MODE=static && make install PREFIX=$PWD/deploy/`uname -m`'
mkdir -p deploy/bin && cp deploy/i686/bin/ckains deploy/bin/ckains.i686

# Summarize files in deploy/bin dir
file deploy/bin/*
