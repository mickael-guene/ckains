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

char *cwd_at_startup;
int main(int argc, char **argv)
{
    int status;

    cwd_at_startup = get_current_dir_name();
    /* parse options */
    status = parse_options(argc, argv);
    if (status) {
        print_usage();
        return status;
    }
    /* now switch world */
    status = launch(argc - optind, argv + optind);

    return status;
}
