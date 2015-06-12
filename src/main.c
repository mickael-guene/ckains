#define _GNU_SOURCE
#include <unistd.h>
#include <getopt.h>

#include "cwd.h"
#include "options.h"

char *cwd_at_startup;
int main(int argc, char **argv)
{
	int status;

	cwd_at_startup = get_current_dir_name();
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
