
#include <linux/list.h>
#include <asm/string.h>
// struct to handle what files we're watching
// Definitely needs name of file, and might as well have name length of the file

struct kenlex_event {
	int id;
};

struct kenlex_watch_info {
	struct kenlex_event fse;
	//u32 mask;
	//int wd;
	u32 sync_cookie;
	int name_len;
	char name[];
};

struct kenlex_watch_list_item {
    struct list_head list_node;
    struct kenlex_watch_info info;
};