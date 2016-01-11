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

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <errno.h>

#include "binding.h"
#include "config.h"
#include "log.h"

static char *strjoin(char *a, char *b)
{
    char *res = malloc(strlen(a) + strlen(b) + 1);

    if (res) {
        strcpy(res, a);
        strcat(res, b);
    }

    return res;
}

static char *strjoin_with_separator(char *a, char *b)
{
    char *res = malloc(strlen(a) + 1 + strlen(b) + 1);

    if (res) {
        strcpy(res, a);
        res[strlen(a)] = '/';
        res[strlen(a)+1] = '\0';
        strcat(res, b);
    }

    return res;
}

static char *source_canonicalization(char *old_rootfs_mount_name, char *source, char *cwd)
{
    /* file canonicalization in original rootfs already done */
    assert(source != NULL && source[0] == '/');
    /* so we just need to concat old_rootfs_mount_name with source */
    return strjoin(old_rootfs_mount_name, source);
}

static char *target_canonicalization(char *target, char *cwd)
{
    char *res;

    if (target[0] == '/') {
        res = strdup(target);
    } else {
        /* FIXME: add support to strip .. in path. This will correct the
         * fact that upper level directory are created.
         */
        res = strjoin_with_separator(cwd, target);
    }

    return res;
}

static void canonicalization(char *old_rootfs_mount_name, char *cwd)
{
    int i;

    for(i = 0; i < config.mounts_nb; i++) {
        config.mounts[i].source_canonicalized = source_canonicalization(old_rootfs_mount_name, config.mounts[i].source, cwd);
        if (!config.mounts[i].source_canonicalized)
            error("Unable to canonicalize source %s\n", config.mounts[i].source);
        config.mounts[i].target_canonicalized = target_canonicalization(config.mounts[i].target, cwd);
        if (!config.mounts[i].target_canonicalized)
            error("Unable to canonicalize target %s\n", config.mounts[i].target);
    }
}

static int compare_elem(const void *a, const void *b)
{
    return strcmp(((struct mount_info *)a)->target_canonicalized, ((struct mount_info *)b)->target_canonicalized);
}

static void sort_by_target()
{
    qsort(config.mounts, config.mounts_nb, sizeof(struct mount_info), compare_elem);
}

static int test_source_exist(char *source, int *is_dir)
{
    struct stat buf;
    int result = stat(source, &buf);

    if(!result) {
        *is_dir = S_ISDIR(buf.st_mode);
    }

    return result;
}

static int create_target_hierarchy(char *target, int is_dir)
{
    int status;
    struct stat buf;

    //fprintf(stderr, "target = %s / is_dir = %d\n", target, is_dir);
    if (!is_dir) {
        int result = stat(target, &buf);

        if (result) {
            /* path doesn't exist so we need to create it */
            char *pos = strrchr(target, '/');
            if (pos != target) {
                *pos = '\0';
                status = create_target_hierarchy(target, 1);
                *pos = '/';
                if (status)
                    return status;
            }
            //fprintf(stderr, "creating file %s\n", target);
            return close(open(target, O_CREAT | O_WRONLY, 0777));
        } else if (!S_ISDIR(buf.st_mode)) {
            /* file exists and is not a directory */
            return 0;
        } else {
            /* path exist and is a directory */
            return -1;
        }
    } else {
        int result = stat(target, &buf);

        if (result) {
            /* path doesn't not exist so we need to create it */
             /* but first insure parent path exists */
            char *pos = strrchr(target, '/');
            if (pos != target) {
                *pos = '\0';
                status = create_target_hierarchy(target, 1);
                *pos = '/';
                if (status)
                    return status;
            }
            //fprintf(stderr, "creating directory %s\n", target);
#if 1
            /* due to the fact that target_canonicalization don't remove ..
             * in path then we might try to create an already created directory.
             * so we handle this case here. But this is only a temporary solution
             * until target_canonicalization support remove of .. in path
            */
            result = mkdir(target, 0777);
            if (result && errno == EEXIST)
                result = 0;
            return result;
#else
            return mkdir(target, 0777);
#endif
        } else if (S_ISDIR(buf.st_mode)) {
            /* path already exist and is a directory */
            return 0;
        } else {
            /* path exist and is not a directory */
            return -1;
        }
    }

    /* should not come here */
    assert(0);
}

static void bind_them_all()
{
    int i;
    int is_dir = 0;

    for(i = 0; i < config.mounts_nb; i++) {
        /* test source exist and return type */
        if (test_source_exist(config.mounts[i].source_canonicalized, &is_dir)) {
            if (config.mounts[i].skip_on_error) {
                info("How this is possible !!! Unable to find %s\n", config.mounts[i].source_canonicalized);
                continue;
            }
            error("How this is possible !!! Unable to find %s\n", config.mounts[i].source_canonicalized);
        }

        /* create destination hierarchy */
        if (create_target_hierarchy(config.mounts[i].target_canonicalized, is_dir)) {
            if (config.mounts[i].skip_on_error) {
                info("Unable to create target mount point %s\n", config.mounts[i].target_canonicalized);
                continue;
            }
            error("Unable to create target mount point %s\n", config.mounts[i].target_canonicalized);
        }

        /* do the bind */
        if (mount(config.mounts[i].source_canonicalized,
                  config.mounts[i].target_canonicalized,
                  NULL, MS_PRIVATE | MS_BIND | MS_REC, NULL)) {
            if (config.mounts[i].skip_on_error) {
                info("Unable to bind %s to %s, failed with error %s\n", config.mounts[i].source_canonicalized, config.mounts[i].target_canonicalized, strerror(errno));
                continue;
            }
            error("Unable to bind %s to %s, failed with error %s\n", config.mounts[i].source_canonicalized, config.mounts[i].target_canonicalized, strerror(errno));
        }
    }
}

void mount_bindings(char *old_rootfs_mount_name, char *cwd)
{
    canonicalization(old_rootfs_mount_name?old_rootfs_mount_name:"", cwd);
    sort_by_target();
    bind_them_all();
}
