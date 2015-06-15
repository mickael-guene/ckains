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

#define _GNU_SOURCE
#include <unistd.h>
#include <getopt.h>

#include "cwd.h"
#include "options.h"
#include "core.h"

char *cwd_at_startup;
int main(int argc, char **argv)
{
    cwd_at_startup = get_current_dir_name();
    /* parse options */
    parse_options(argc, argv);

    /* now switch world */
    launch(argc - optind, argv + optind);

    /* launch should not return. either it exec or it exit with error */
    return -1;
}
