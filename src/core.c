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
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/mount.h>
#include <sys/personality.h>
#include <errno.h>
#include <stdlib.h>

#include "core.h"
#include "config.h"
#include "cwd.h"
#include "binding.h"
#include "log.h"

static void update_id(char *filename, int id_old, int id_new)
{
    int status;
    char cmdline[PATH_MAX];
    int fd = open(filename, O_WRONLY);

    if (fd >= 0) {
        snprintf(cmdline, sizeof(cmdline), "%d %d 1", id_new, id_old);
        status = write(fd, cmdline, strlen(cmdline));
        close(fd);
        if (status < 0)
            error("write of %s in %s failed with error: %s\n", cmdline, filename, strerror(errno));
    } else
        error("open of %s failed with error: %s\n", filename, strerror(errno));
}

static void setup_setgroups()
{
    char cmdline[PATH_MAX];
    int fd = open("/proc/self/setgroups", O_WRONLY);

    if (fd >= 0) {
        snprintf(cmdline, sizeof(cmdline), "deny");
        write(fd, cmdline, strlen(cmdline));
        close(fd);
    }
}

static void setup_mapping(uid_t uid_old, gid_t gid_old)
{
    uid_t uid_new = config.is_root_id?0:uid_old;
    gid_t gid_new = config.is_root_id?0:gid_old;

    update_id("/proc/self/uid_map", uid_old, uid_new);
    setup_setgroups();
    update_id("/proc/self/gid_map", gid_old, gid_new);
}

static int umount_original_rootfs(char *mount_name)
{
    if (mount_name) {
        if (umount2(mount_name, MNT_DETACH))
            return -1;
        if (rmdir(mount_name))
            return -1;
    }

    return 0;
}

static void umount_original_rootfs_on_exit(int exit_no, void *arg)
{
    umount_original_rootfs((char *) arg);
}

static char *mount_original_rootfs()
{
    char mount_name_old_rootfs[PATH_MAX];
    static char mount_name_new_rootfs[PATH_MAX];

    if (strcmp("/", config.rootfs) == 0) {
        return NULL;
    } else {
        snprintf(mount_name_old_rootfs, sizeof(mount_name_old_rootfs), "%s/ckains-%d", config.rootfs, getpid());
        snprintf(mount_name_new_rootfs, sizeof(mount_name_new_rootfs), "/ckains-%d", getpid());
    }

    if (mkdir(mount_name_old_rootfs, 0755))
        error("unable to create %s with error: %s\n", mount_name_old_rootfs, strerror(errno));
    if (mount("/", mount_name_old_rootfs, NULL, MS_PRIVATE | MS_BIND | MS_REC, NULL)) {
        rmdir(mount_name_old_rootfs);
        error("unable to mount %s with error: %s\n", mount_name_old_rootfs, strerror(errno));
    }

    if (on_exit(umount_original_rootfs_on_exit, mount_name_new_rootfs))
        error("unable to register umount_original_rootfs with error %s\n", strerror(errno));

    return mount_name_new_rootfs;
}

void launch(int argc, char **argv)
{
    uid_t uid = getuid();
    gid_t gid = getgid();
    char *tmp_mount_name;
    int unshare_flags = CLONE_NEWUSER | CLONE_NEWNS;

    /* let's enter new world */
    if (config.hostname)
        unshare_flags |= CLONE_NEWUTS;
    if (unshare(unshare_flags))
        error("unshare failed with error %s\n", strerror(errno));

    /* set hostname */
    if (config.hostname) {
        if (sethostname(config.hostname, strlen(config.hostname)))
            error("sethostname failed with error %s\n", strerror(errno));
    }

    /* setup id mapping table */
    setup_mapping(uid, gid);

    /* temporary mount original rootfs */
    tmp_mount_name = mount_original_rootfs();

    /* change rootfs */
    if (chroot(config.rootfs))
        error("chroot into %s failed with error %s\n", config.rootfs, strerror(errno));

    /* bind mounts requested dir */
    mount_bindings(tmp_mount_name, cwd_at_startup);

    /* now we can safely umount original rootfs */
    if (umount_original_rootfs(tmp_mount_name))
        error("Unable to umount %s\n", tmp_mount_name);

    /* jump to required cwd */
    if (chdir(config.cwd)) {
        warning("Failed to change the working directory to '%s': does not exist (in the new rootfs).\n", config.cwd);
        warning("Changing the current working directory to \"/\".\n");
        if (chdir("/"))
            error("Unable to change to \"/\"\n");
    }

    /* change personality if required */
    if (config.is_32_bit_mode)
        personality(PER_LINUX32);

    if (argv[0])
        execvp(argv[0], argv);
    else {
        char *no_argv[] = {
            "/bin/sh",
            "-l",
            NULL
        };
        execvp(no_argv[0], no_argv);
    }

    error("exec of %s failed with error %s\n", argv[0]?argv[0]:"bin/sh", strerror(errno));
}
