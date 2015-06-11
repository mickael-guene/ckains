#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>

#include "options.h"
#include "config.h"

struct config config;
static struct option long_options[] = {
    {"rootfs",          required_argument, NULL, 'r'},
    {"root-id",         no_argument, NULL, '0'},
    {"bind",            required_argument, NULL, 'b'}
};

static void setup_default_config(struct config *config)
{
    config->rootfs = "/";
    config->is_root_id = 0;
    config->mounts_nb = 0;
}

static void append_mount_point(struct config *config, char *source, char *target)
{
    assert(config->mounts_nb != MAX_MOUNT_INFO_NB);
    config->mounts[config->mounts_nb].source = source;
    config->mounts[config->mounts_nb].target = target;
    config->mounts_nb++;
}

int parse_options(int argc, char **argv)
{
    int opt;

    setup_default_config(&config);
    while((opt = getopt_long(argc, argv, "+r:0b:", long_options, NULL)) != -1) {
        switch(opt) {
            case 'r':
                config.rootfs = optarg;
                break;
            case '0':
                config.is_root_id = 1;
                break;
            case 'b':
                append_mount_point(&config, optarg, optarg);
                break;
            default:
                return -1;
        }
    }

    return 0;
}

void print_usage()
{
    fprintf(stderr, "ckains : fixme\n");
}
