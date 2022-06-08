#include "logger.h"
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/uaccess.h>	/* for copy_*_user */

#define PROC_DIR "logger"
#define PROC_FILENAME "buffer"

#define ATRW atomic_read ( &core.wHead )
#define ATRR atomic_read ( &core.rHead )

static struct proc_dir_entry *procfs_dir=NULL;
static struct proc_dir_entry *procfs_file=NULL;

static ssize_t procfs_read(struct file *file, char __user *ubuf, size_t bufLen, loff_t *ppos);

static struct proc_ops file_ops={
	.proc_read	= procfs_read,
};

static ssize_t procfs_read(struct file *file, char __user *ubuf, size_t bufLen, loff_t *ppos) {
	unsigned long res;
	ssize_t len = ( ATRW + core.capacity - ATRR ) % core.capacity - 1;
	ssize_t toCopy;
	int i;
	char buffer[ 10 ];
	printk(KERN_ERR "ATR WR %d %d @ %d\n", ATRW, ATRR, len );
	
	if ( file==NULL || ubuf==NULL || ppos==NULL ) {
		return -EFAULT;			
	}
	if( *ppos < 0 || *ppos > len ) { return -EBADFD; }
	if( *ppos == len || len == 0 ) { return 0; }
	
	toCopy = len - *ppos;
	if ( toCopy > bufLen ) {
		toCopy = bufLen;
	}
	for ( i = 0; i < toCopy; i++ ) {
		if ( ATRW == ATRR ) {
			toCopy = i;
			break;
		} else {
			buffer [ i ] = core.payload [ ATRR ];
			printk(KERN_ERR "Byte: %d\n", (int)buffer[ i ] );
			atomic_set ( &core.rHead, ( ATRR + 1 ) % core.capacity );
		}
	}
	printk(KERN_ERR "B: %s\n", buffer );
	res=copy_to_user ( ubuf, buffer, toCopy );
	if(res) {
		printk(KERN_ERR "procfs_read - could not copy data to userspace\n");
		return -ENOMEM;			
	}
	*ppos+=toCopy;
	return toCopy;
}

int procFsInit() {
	if(procfs_dir!=NULL) {
		printk(KERN_ALERT "Error: Directory /proc/%s already exists\n",PROC_DIR);
		return -EEXIST;
	}	
	procfs_dir=proc_mkdir(PROC_DIR,NULL);
	if(procfs_dir==NULL) {
		printk(KERN_ALERT "Error: Could not create directory /proc/%s\n",PROC_DIR);
		return -ENOENT;
	}	
	procfs_file=proc_create(PROC_FILENAME,0644,procfs_dir,&file_ops);	
	if(procfs_file==NULL){
		proc_remove(procfs_dir);	
		procfs_dir=NULL;
		printk(KERN_ALERT "Error: Could not create file /proc/%s/%s\n",PROC_DIR,PROC_FILENAME);
		return -ENOENT;
	}	
	printk(KERN_INFO "Created /proc/%s/%s\n",PROC_DIR,PROC_FILENAME);
	return 0;
}

void procFsCleanup() {
	if(procfs_dir!=NULL) {
		remove_proc_entry(PROC_FILENAME,procfs_dir);
		remove_proc_entry(PROC_DIR,NULL);
	} else {
		printk(KERN_ALERT "Error: Directory /proc/%s not existing\n",PROC_DIR);
	}
}
