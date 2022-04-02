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
// Like most kernel stuff, there will be an interface file in /proc which will (for now) only contain the path to files to watch

// Link for how to interface with proc
// https://devarea.com/linux-kernel-development-creating-a-proc-file-and-interfacing-with-user-space/#.YkNIp5rMJb8



// HOW VIRTUAL FILE SYSTEM WORKS
// Whenever the file is written to, it will call mywrite. Which can see if the thing written to the file is an existing file we're watching
// or if we need to add it to our struct list 
// Whenever someone tries to read from our file, it will call myread, and we should (for now) just print out the names of the files in our struct list


// kernel/list Linked list guides:
// https://tuxthink.blogspot.com/2014/02/creating-linked-list-in-liinux-kernel.html
// https://medium.com/@414apache/kernel-data-structures-linkedlist-b13e4f8de4bf


//the file we'll use to interface with userspace
static struct proc_dir_entry *kenlex_file_info;

// list_head is defined in <linux/list.h>
// head of our linked list containing file paths to watch
struct list_head kenlex_watch_list_head;
 
// TODO: how to use module_param? 

// For our code, we're really going to be reading in strings-- File name paths and creating structs for what we're monitoring,
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	int num,c,i,m;
	char buf[BUFSIZE];
	char newPath[count];
	if(count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf, ubuf, count))
		return -EFAULT;
	num = sscanf(buf,"%s",newPath);
	// only scanning in 1 thing (the file path), in future will scan in path and likely some numbers to define montioring settings
	if(num != 1)
		return -EFAULT;
	
	c = strlen(buf);
	new_node->info.name = (char*)kmalloc(c,GFP_KERNEL)
	new_node->info.name_len = c;
	strcpy(new_node->info.name, buf);
	list_add(&new_node->list_node,&kenlex_watch_list_head);
	// TODO: add new file to watch
	*ppos = c;
	return c;
}
 
// function called when someone attempts to read from our virtual file
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	char buf[BUFSIZE];
	int len=0;
	if(*ppos > 0 || count < BUFSIZE)
		return 0;
	
	struct list_head *ptr;
	struct kenlex_watch_list_item *current_entry;
	// Question: Is this good enough?? What happens when we go past buff limit? is this thought through?
	// Answer: out of scope, limit number of watched files
	list_for_each(ptr, &kenlex_watch_list_head) {
 		current_entry=list_entry(ptr,struct kenlex_watch_list_item,list_node);
		// printing each file name to buffer
       	len  += sprintf(buf+len, "\n%s\n",current_entry->info.name);
  	}
	// showing the list to the user
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
		struct kenlex_watch_list_item *etc_password;
        //kenlex_file_info = proc_create("kenlex", 0660, NULL, &myops);
        printk (KERN_INFO "Kenlex Enabled\n");

		setup_kenlex_monitor();
		INIT_LIST_HEAD(&kenlex_watch_list_head);

		//allocate memory for included-by-default files in our watch list
		etc_password=kmalloc(sizeof(struct kenlex_watch_list_item *),GFP_KERNEL);

		//assign values for included-by-default files in our watch list
		etc_password->info.name = (char*)kmalloc(sizeof("/etc/password"),GFP_KERNEL)
		etc_password->info.name_len = strlen("etc/password")
		strcpy(etc_password->info.name, "/etc/password");

		// Add the default files to the list
		list_add(&etc_password->list_node,&kenlex_watch_list_head);

		//FIXME: loop through list to add all file paths to the inotify watch
		// Question.... do we need to handle the context? Like which user added it?
		
		
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