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
    char *res;

    if (source[0] == '/') {
        res = strjoin(old_rootfs_mount_name, source);
    } else {
        char *tmp;
        char *tmp2;

        tmp = strjoin_with_separator(cwd, source);
        tmp2 = strjoin(old_rootfs_mount_name, tmp);
        res = realpath(tmp2, NULL);

        free(tmp);
        free(tmp2);
    }

    return res;
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
        //fprintf(stderr, "%s = > %s / %s\n", target, res, realpath(res, NULL));
    }

    return res;
}

static int canonicalization(char *old_rootfs_mount_name, char *cwd)
{
    int i;

    for(i = 0; i < config.mounts_nb; i++) {
        config.mounts[i].source_canonicalized = source_canonicalization(old_rootfs_mount_name, config.mounts[i].source, cwd);
        if (!config.mounts[i].source_canonicalized)
            return -1;
        config.mounts[i].target_canonicalized = target_canonicalization(config.mounts[i].target, cwd);
        if (!config.mounts[i].target_canonicalized)
            return -1;
        //fprintf(stderr, "%s -> %s\n", config.mounts[i].source_canonicalized, config.mounts[i].target_canonicalized);
    }

    return 0;
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

static int bind_them_all()
{
    int i;
    int is_dir;
    int status;

    for(i = 0; i < config.mounts_nb; i++) {
        /* test source exist and return type */
        status = test_source_exist(config.mounts[i].source_canonicalized, &is_dir);
        if (status) {
            if (config.mounts[i].skip_on_error)
                continue;
            fprintf(stderr, "%s DOEST NOT exist\n", config.mounts[i].source_canonicalized);
            return status;
        }
        //fprintf(stderr, "%s exist and is_dir = %d\n", config.mounts[i].source_canonicalized, is_dir);

        /* create destination hierarchy */
        status = create_target_hierarchy(config.mounts[i].target_canonicalized, is_dir);
        if (status)
            return status;

        /* do the bind */
        status = mount(config.mounts[i].source_canonicalized,
                       config.mounts[i].target_canonicalized,
                       NULL,
                       MS_PRIVATE | MS_BIND | MS_REC,
                       NULL);
        if (status)
            return status;
    }

    return 0;
}

int mount_bindings(char *old_rootfs_mount_name, char *cwd)
{
    int status;
    int i;

    status = canonicalization(old_rootfs_mount_name, cwd);
    if (status)
        return status;
    sort_by_target();
    status = bind_them_all();
    if (status)
        return status;

    return 0;
}
