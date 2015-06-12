#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <stdlib.h>

#include "options.h"
#include "config.h"
#include "cwd.h"

struct config config;
static struct option long_options[] = {
    {"rootfs",              required_argument, NULL, 'r'},
    {"root-id",             no_argument, NULL, '0'},
    {"bind",                required_argument, NULL, 'b'},
    {"mount",               required_argument, NULL, 'm'},
    {"bind-elsewhere",      required_argument, NULL, 'B'},
    {"mount-elsewhere",     required_argument, NULL, 'M'},
    {"pwd",                 required_argument, NULL, 'w'},
    {"cwd",                 required_argument, NULL, 'w'},
    {"working-directory",   required_argument, NULL, 'w'},
    {"32",                  no_argument, NULL, 'P'},
    {"32bit",               no_argument, NULL, 'P'},
    {"32bit-mode",          no_argument, NULL, 'P'},
    {"verbose",             no_argument, NULL, 'v'},
    {"help",                no_argument, NULL, 'h'},
    {"usage",               no_argument, NULL, 'h'},
    {"hostname",            required_argument, NULL, 'n'},
};

static char *R_bindings[] = {
    "/etc/host.conf",
    "/etc/hosts",
    "/etc/hosts.equiv",
    "/etc/mtab",
    "/etc/netgroup",
    "/etc/networks",
    "/etc/passwd",
    "/etc/group",
    "/etc/nsswitch.conf",
    "/etc/resolv.conf",
    "/etc/localtime",
    "/dev/",
    "/sys/",
    "/proc/",
    "/tmp/",
    "/run/"
};

static char *S_bindings[] = {
    "/etc/host.conf",
    "/etc/hosts",
    "/etc/nsswitch.conf",
    "/etc/resolv.conf",
    "/dev/",
    "/sys/",
    "/proc/",
    "/tmp/",
    "/run/shm",
};

static void setup_default_config(struct config *config)
{
    config->rootfs = "/";
    config->is_root_id = 0;
    config->mounts_nb = 0;
    config->cwd = cwd_at_startup;
    config->is_32_bit_mode = 0;
    config->is_verbose = 0;
    config->hostname = NULL;
}

static void append_mount_point(struct config *config, char *source, char *target, int skip_on_error)
{
    assert(config->mounts_nb != MAX_MOUNT_INFO_NB);
    config->mounts[config->mounts_nb].skip_on_error = skip_on_error;
    config->mounts[config->mounts_nb].source = source;
    config->mounts[config->mounts_nb].target = target;
    config->mounts_nb++;
}

int parse_options(int argc, char **argv)
{
    int opt;
    int i;

    setup_default_config(&config);
    while((opt = getopt_long(argc, argv, "+r:0b:m:B:M:w:R:S:vhn:", long_options, NULL)) != -1) {
        switch(opt) {
            case 'r':
                config.rootfs = optarg;
                break;
            case '0':
                config.is_root_id = 1;
                break;
            case 'm':
            case 'b':
                append_mount_point(&config, optarg, optarg, 0);
                break;
            case 'M':
            case 'B':
                /* As getopt has no support for multi parameter options we do
                 * some operation that are dependent of implementation and so
                 * fragile. incrementing optind to jump to next option is not
                 * sure to work everywhere. Other solution is to implement a
                 * custom parser ......
                 */
                if (argv[optind][0] == '-')
                    return -1;
                append_mount_point(&config, optarg, argv[optind], 0);
                optind++;
                break;
            case 'w':
                if (optarg[0] == '/')
                    config.cwd = optarg;
                else {
                    /* FIXME: replace this stuff with a resolve_path function also need in binding.c */
                    config.cwd = canonicalize_file_name(optarg);
                    if (!config.cwd)
                        config.cwd = "/";
                }
                break;
            case 'P':
                config.is_32_bit_mode = 1;
                break;
            case 'R':
                config.rootfs = optarg;
                for(i = 0; i < sizeof(R_bindings) / sizeof(R_bindings[0]); i++)
                    append_mount_point(&config, R_bindings[i], R_bindings[i], 1);
                append_mount_point(&config, getenv("HOME"), getenv("HOME"), 1);
                break;
            case 'S':
                config.rootfs = optarg;
                config.is_root_id = 1;
                for(i = 0; i < sizeof(S_bindings) / sizeof(S_bindings[0]); i++)
                    append_mount_point(&config, S_bindings[i], S_bindings[i], 1);
                append_mount_point(&config, getenv("HOME"), getenv("HOME"), 1);
                break;
            case 'v':
                config.is_verbose = 1;
                break;
            case 'h':
                print_usage();
                exit(1);
                break;
            case 'n':
                config.hostname = optarg;
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
