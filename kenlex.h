
#include <linux/list.h>
#include <asm/string.h>
// struct to handle what files we're watching
// Definitely needs name of file, and might as well have name length of the file

// what events we watch for. Could be edited to include frequencies, and low/high priority
struct kenlex_event {
	int id;
};

// defines different files being watched, modeled after inotify struct
struct kenlex_watch_info {
	struct kenlex_event fse;
	//u32 mask;
	//int wd;
	u32 sync_cookie;
	int name_len;
	char name[];
};

// used to build linked list of all settings
struct kenlex_watch_list_item {
    struct list_head list_node;
    struct kenlex_watch_info info;
};