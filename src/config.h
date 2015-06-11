#ifndef __CONFIG__H__
#define __CONFIG__H__

struct config {
	char *rootfs;
	int is_root_id;
};
extern struct config config;

#endif
