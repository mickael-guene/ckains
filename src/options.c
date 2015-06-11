#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include "options.h"
#include "config.h"

struct config config;
static struct option long_options[] = {
    {"rootfs",          required_argument, NULL, 'r'},
    {"root-id",         no_argument, NULL, '0'}
};

static void setup_default_config(struct config *config)
{
    config->rootfs = "/";
    config->is_root_id = 0;
}

int parse_options(int argc, char **argv)
{
    int opt;

    setup_default_config(&config);
    while((opt = getopt_long(argc, argv, "+r:0", long_options, NULL)) != -1) {
        switch(opt) {
            case 'r':
                config.rootfs = optarg;
                break;
            case '0':
                config.is_root_id = 1;
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
