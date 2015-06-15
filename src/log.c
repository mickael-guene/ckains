/* This file is part of ckains, a c clone of Kains.
 *
 * Copyright (C) 2015 STMicroelectronics
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"

void debug(const char *format, ...) {
    va_list args;

    if (config.is_verbose > 1) {
        va_start(args, format);
        fprintf(stderr, "DEBUG: ");
        vfprintf(stderr, format, args);
        va_end(args) ;
    }
}

void info(const char *format, ...) {
    va_list args;

    if (config.is_verbose > 1) {
        va_start(args, format);
        fprintf(stderr, "INFO: ");
        vfprintf(stderr, format, args);
        va_end(args) ;
    }
}

void warning(const char *format, ...) {
    va_list args;

    if (config.is_verbose) {
        va_start(args, format);
        fprintf(stderr, "WARNING: ");
        vfprintf(stderr, format, args);
        va_end(args) ;
    }
}

void error(const char *format, ...) {
    va_list args;

    va_start(args, format);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
    va_end(args) ;

    exit(-1);
}
