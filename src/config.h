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

#ifndef __CONFIG__H__
#define __CONFIG__H__

#define MAX_MOUNT_INFO_NB       256

struct mount_info {
    int skip_on_error;
    char *source;
    char *target;
    char *source_canonicalized;
    char *target_canonicalized;
};

struct config {
    char *rootfs;
    int is_root_id;
    int mounts_nb;
    struct mount_info mounts[MAX_MOUNT_INFO_NB];
    char *cwd;
    int is_32_bit_mode;
    int is_verbose;
    char *hostname;
};

extern struct config config;

#endif
