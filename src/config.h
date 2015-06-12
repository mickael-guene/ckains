#ifndef __CONFIG__H__
#define __CONFIG__H__

#define MAX_MOUNT_INFO_NB       256

struct mount_info {
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
};

extern struct config config;

#endif
