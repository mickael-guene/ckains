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

#include "core.h"
#include "config.h"
#include "cwd.h"

static int update_id(char *filename, int id_old, int id_new)
{
    int status;
    char cmdline[PATH_MAX];
    int fd = open(filename, O_WRONLY);

    if (fd >= 0) {
        snprintf(cmdline, sizeof(cmdline), "%d %d 1", id_new, id_old);
        status = write(fd, cmdline, strlen(cmdline));
        close(fd);
        if (status < 0)
            return -1;
    } else
        return -1;

    return 0;
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

static int setup_mapping(uid_t uid_old, gid_t gid_old)
{
    int status;
    uid_t uid_new = config.is_root_id?0:uid_old;
    gid_t gid_new = config.is_root_id?0:gid_old;

    status = update_id("/proc/self/uid_map", uid_old, uid_new);
    if (status)
        return status;
    setup_setgroups();
    status = update_id("/proc/self/gid_map", gid_old, gid_new);
    if (status)
        return status;

    return 0;
}

static char *mount_original_rootfs()
{
    int status;
    char mount_name_old_rootfs[PATH_MAX];
    static char mount_name_new_rootfs[PATH_MAX];

    if (strcmp("/", config.rootfs) == 0) {
        return NULL;
    } else {
        snprintf(mount_name_old_rootfs, sizeof(mount_name_old_rootfs), "%s/ckains-%d", config.rootfs, getpid());
        snprintf(mount_name_new_rootfs, sizeof(mount_name_new_rootfs), "/ckains-%d", getpid());
    }

    status = mkdir(mount_name_old_rootfs, 0755);
    if (status)
        return NULL;
    status = mount("/", mount_name_old_rootfs, NULL, MS_PRIVATE | MS_BIND | MS_REC, NULL);
    if (status) {
        rmdir(mount_name_old_rootfs);
        return NULL;
    }

    return mount_name_new_rootfs;
}

static int umount_original_rootfs(char *mount_name)
{
    int status;

    if (mount_name) {
        status = umount2(mount_name, MNT_DETACH);
        if (status)
            return status;
        status = rmdir(mount_name);
        if (status)
            return status;
    }

    return 0;
}

int launch(int argc, char **argv)
{
    int status;
    uid_t uid = getuid();
    gid_t gid = getgid();
    char *tmp_mount_name;

    /* let's enter new world */
    status = unshare(CLONE_NEWUSER | CLONE_NEWNS);
    if (status)
        return status;

    /* setup id mapping table */
    status = setup_mapping(uid, gid);
    if (status)
        return status;

    /* temporary mount original rootfs */
    tmp_mount_name = mount_original_rootfs();

    /* change rootfs */
    status = chroot(config.rootfs);
    if (status)
        return status;

    /* bind mounts requested dir */
    status = mount_bindings(tmp_mount_name, cwd_at_startup);
    if (status) {
        umount_original_rootfs(tmp_mount_name);
        return status;
    }

    /* now we can safely umount original rootfs */
    status = umount_original_rootfs(tmp_mount_name);
    if (status)
        return status;

    /* jump to required cwd */
    status = chdir(config.cwd);
    if (status) {
        fprintf(stderr, "Failed to change the working directory to '%s': does not exist (in the new rootfs).\n", config.cwd);
        fprintf(stderr, "Changing the current working directory to \"/\".\n");
        status = chdir("/");
        if (status)
            return status;
    }

    /* change personality if required */
    if (config.is_32_bit_mode)
        personality(PER_LINUX32);

    execvp(argv[0], argv);

    return -1;
}
