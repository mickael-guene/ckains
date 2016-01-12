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
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "options.h"
#include "config.h"
#include "cwd.h"
#include "log.h"

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
    {"version",             no_argument, NULL, 'V'},
    {NULL, 0, NULL, 0}
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
    config->is_verbose = 1;
    config->hostname = NULL;
}

static void append_mount_point(struct config *config, char *source, char *target, int skip_on_error)
{
    assert(config->mounts_nb != MAX_MOUNT_INFO_NB);
    config->mounts[config->mounts_nb].skip_on_error = skip_on_error;
    /* we resolve path before world switch */
    config->mounts[config->mounts_nb].source = canonicalize_file_name(source);
    config->mounts[config->mounts_nb].target = target;
    if (!config->mounts[config->mounts_nb].source) {
        /* stop here unless skip_on_error is true */
        if (!skip_on_error)
            error("Unknown source mount point %s\n", source);
        else
            info("Unknown source mount point %s\n", source);
    } else
       config->mounts_nb++; 
}

static void set_rootfs(char *rootfs)
{
    struct stat buf;

    if (stat(rootfs, &buf))
        error("'%s' doesn't exist\n", rootfs);
    else if (!S_ISDIR(buf.st_mode))
        error("'%s' isn't a directory\n", rootfs);

    config.rootfs = canonicalize_file_name(rootfs);
}

const char rootfs_usage[] = "\
  -r, --rootfs [ROOTFS]             use ROOTFS as the new root file-system\n";
const char root_id_usage[] = "\
  -0, --root-id                     Set user and group identities virtually to \n\
                                    \"root/root\"\n";
const char bind_usage[] = "\
  -b, --bind [PATH]                 Make PATH visible from the virtual rootfs, at \n\
                                    the same location\n";
const char mount_usage[] = "\
  -m, --mount [PATH]                Alias for --bind option\n";
const char bind_elsewhere_usage[] = "\
  -B, --bind-elsewhere [PATH] [LOCATION]    Make PATH visible from the virtual\n\
                                rootfs, at the given LOCATION\n";
const char mount_elsewhere_usage[] = "\
  -M, --mount-elsewhere [PATH] [LOCATION]   Alias for --bind-elsewhere option\n";
const char working_directory_usage[] = "\
  -w, --working-directory [PATH]    Set the initial working directory to PATH\n";
const char pwd_usage[] = "\
      --pwd [PATH]                  Alias for --working-directory option\n";
const char cwd_usage[] = "\
      --cwd [PATH]                  Alias for --working-directory option\n";
const char bitmode_32_usage[] = "\
      --32bit-mode                  Make Linux declare itself and behave as a \n\
                                    32-bit kernel\n";
const char bitmode_32_usage_2[] = "\
      --32bit                       Alias for --32bit-mode option\n";
const char bitmode_32_usage_3[] = "\
      --32                          Alias for --32bit-mode option\n";
const char R_usage[] = "\
  -R [PATH]                         Use PATH as virtual rootfs + bind some \n\
                                    files/directories.\n";
const char S_usage[] = "\
  -S [PATH]                         Use PATH as virtual rootfs + bind some \n\
                                    files/directories. + fake \"root\"\n";
const char hostname_usage[] = "\
  -n, --hostname [HOSTNAME]         Set HOSTNAME as the new name\n";

const char help_usage[] = "\
  -h, --help                        Print the help message, then exit\n";
const char usage_usage[] = "\
      --usage                       Alias for --help\n";
const char version_usage[] = "\
      --version                     Output version information and exit\n";

static void print_usage(char *name)
{
    fprintf(stderr, "Usage: %s [OPTION] ... [COMMAND]\n\n", name);
    fprintf(stderr, rootfs_usage);
    fprintf(stderr, root_id_usage);
    fprintf(stderr, bind_usage);
    fprintf(stderr, mount_usage);
    fprintf(stderr, bind_elsewhere_usage);
    fprintf(stderr, mount_elsewhere_usage);
    fprintf(stderr, working_directory_usage);
    fprintf(stderr, pwd_usage);
    fprintf(stderr, cwd_usage);
    fprintf(stderr, bitmode_32_usage);
    fprintf(stderr, bitmode_32_usage_2);
    fprintf(stderr, bitmode_32_usage_3);
    fprintf(stderr, R_usage);
    fprintf(stderr, S_usage);
    fprintf(stderr, hostname_usage);
    fprintf(stderr, help_usage);
    fprintf(stderr, usage_usage);
    fprintf(stderr, version_usage);
}

static void bad_usage(char *name)
{
    fprintf(stderr, "Try '%s --help' for more information.\n", name);
}

static void print_version()
{
    fprintf(stderr, "ckains %s (%s)\n", GIT_VERSION, GIT_DESCRIBE);
}

void parse_options(int argc, char **argv)
{
    int opt;
    int i;

    setup_default_config(&config);
    while((opt = getopt_long(argc, argv, "+r:0b:m:B:M:w:R:S:vhn:V", long_options, NULL)) != -1) {
        switch(opt) {
            case 'r':
                set_rootfs(optarg);
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
                    error("-B second argument must not begin with -, got '%s'\n", argv[optind]);
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
                set_rootfs(optarg);
                for(i = 0; i < sizeof(R_bindings) / sizeof(R_bindings[0]); i++)
                    append_mount_point(&config, R_bindings[i], R_bindings[i], 1);
                append_mount_point(&config, getenv("HOME"), getenv("HOME"), 1);
                break;
            case 'S':
                set_rootfs(optarg);
                config.is_root_id = 1;
                for(i = 0; i < sizeof(S_bindings) / sizeof(S_bindings[0]); i++)
                    append_mount_point(&config, S_bindings[i], S_bindings[i], 1);
                append_mount_point(&config, getenv("HOME"), getenv("HOME"), 1);
                break;
            case 'v':
                config.is_verbose++;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
                break;
            case 'n':
                config.hostname = optarg;
                break;
            case 'V':
                print_version();
                exit(0);
                break;
            default:
                bad_usage(argv[0]);
                exit(-1);
        }
    }
}
