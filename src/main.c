#include <getopt.h>

#include "options.h"

int main(int argc, char **argv)
{
    int status;

    /* parse options */
    status = parse_options(argc, argv);
    if (status) {
        print_usage();
        return status;
    }
    /* now switch world */
    status = launch(argc - optind, argv + optind);

    return status;
}
