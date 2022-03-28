#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include "kenlex.h"
#include "kenlex_monitor.h"

// may not need this, was part of guide for implementing a virtual file
#define BUFSIZE 2048

// Notes
// This module will get basic information from inotify
// And, if the user turns on the setting, will get advanced information (unsuccessful access/write attempts) from the fs code
// Like most kernel stuff, there will be an interface file in /proc which will (for now) only contain the path to files to watch
// When the module is unloaded, the file should be removed from proc
// When the system is rebooted, proc files are automatically trashed
// So, ideally, we should find some way to persist the watched files config across boots?
// But that's a *to-do-later* problem or maybe it's something we just want to ignore altogether (and can blame the users; say they should write scripts to edit it each time)

// Link for how to interface with proc
// g-a-proc-file-and-interfacing-with-user-space/#.Yju1kJrMJb8

// TO DO LIST
// Add the actual kernel code to this repo so we can edit the things we need to edit (fs.c pretty much, also adding a settings option)
// Define struct for storing our info (this is info about files we are watching and what we are watching for, not info about what happens to those files)
// on init add etc/password to the linked list
// Edit the myread and mywrite functions (descriptions following) THESE ARE FOR THE INTERFACE FILE, NOT THE FILE WHERE WE ARE OUTPUTTING EVENTS TO THAT THE EMAIL MODULE WILL READ
// .... Figure out how to get information about what's happening to the files
// Will we have an inotify instance? Will we just copy the code inotify uses? If we use an inotify instance, we'll have to continuously be watching for data added to
// the inotify file, which would mean a kernel thread (i think) and that's not the end of the world. It's likely the best way
// Edit fs.c to call a function here whenever a setting is turned on. Several steps
        // Add a setting to linux settings (in textbook)
        // figure out how to have the kernel call a function in this module (aka, we need to call a function that gives the address of our functions. kill me): https://stackoverflow.com/questions/25497069/how-to-call-a-function-defined-in-a-kernel-module-lkm-from-kernel-code
        // If the setting is turned on, then, in fs.c when a write/read/open permission is denied, call our function


// HOW VIRTUAL FILE SYSTEM WORKS
// Whenever the file is written to, it will call mywrite. Which can see if the thing written to the file is an existing file we're watching
// or if we need to add it to our struct list 
// Whenever someone tries to read from our file, it will call myread, and we should (for now) just print out the names of the files in our struct list
// Key question: How do manage the struct list? Linked list, probably - https://tuxthink.blogspot.com/2014/02/creating-linked-list-in-liinux-kernel.html
// https://medium.com/@414apache/kernel-data-structures-linkedlist-b13e4f8de4bf

// Need a definition for our struct (for now, it likely is just a file path, in the future, it will be more things including importance, what we're watching for, etc)


//the file we'll use to interface with userspace
static struct proc_dir_entry *kenlex_file_info;
 
//this stuff is based on the example guide i found, we will not be using it
static int irq=20;
module_param(irq,int,0660);
 
static int mode=1;
module_param(mode,int,0660);

// FIXME: change this code to take info from ubuf and use it to add structs to our linked list (for now, just file paths)
// For our code, we're really going to be reading in strings-- File name paths and creating structs for what we're monitoring,
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	int num,c,i,m;
	char buf[BUFSIZE];
	if(*ppos > 0 || count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf, ubuf, count))
		return -EFAULT;
	num = sscanf(buf,"%d %d",&i,&m);
	if(num != 2)
		return -EFAULT;
	irq = i; 
	mode = m;
	c = strlen(buf);
	*ppos = c;
	return c;
}
 
 // edit to be relevant to our program (current code is from an online guide)
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	char buf[BUFSIZE];
	int len=0;
	if(*ppos > 0 || count < BUFSIZE)
		return 0;
	len += sprintf(buf,"irq = %d\n",irq);
	len += sprintf(buf + len,"mode = %d\n",mode);
	
	if(copy_to_user(ubuf,buf,len))
		return -EFAULT;
	*ppos = len;
	return len;
}
 
static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
};

static int kenlex_init(void) {

        //kenlex_file_info = proc_create("kenlex", 0660, NULL, &myops);
        printk (KERN_INFO "Kenlex Enabled\n");

		setup_kenlex_monitor();

		/*
		int kwd1 = kenlex_add_path(path1);
		int kwd2 = kenlex_add_path(path2);
		int kwd3 = kenlex_add_path(path3);

		listen_to_kenlex_events(kwd1);
		listen_to_kenlex_events(kwd2);
		*/

		kenlex_cleanup();

        return 0;
}

static void kenlex_exit(void) {
	kenlex_cleanup();
}

module_init(kenlex_init);
module_exit(kenlex_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kenneth Sherman and Alex Myers");
MODULE_DESCRIPTION("Kenlex File Security Module");