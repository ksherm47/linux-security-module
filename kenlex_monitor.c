#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/kthread.h>

static int inotify_fd = -1;
static int* inotify_wd_list;
static int wd_size = 0;
static int wd_max_size = 10;

static struct task_struct** kenlex_thread_list;

int setup_kenlex_monitor() {
    inotify_fd = inotify_init1(0);
    if (inotify_fd < 0) {
        printk("Error opening inotify instance: Error %d\n", errno);
        return -1;
    }

    inotify_wd_list = (int*)malloc(wd_max_size * sizeof(int));
    kenlex_thread_list = (struct task_struct**)malloc(wd_max_size * sizeof(task_struct*))
    return 0;
}

int listen_for_kenlex_events(int kenlex_wd) {
    char* thread_name_prefix = "kenlex_thread_";
    char thread_name[strlen(thread_name_prefix) + 4];
    sprintf(thread_name, "kenlex_thread_%d", kenlex_wd);
    
    kenlex_thread_list[kenlex_wd] = kthread_create(listen, (void*)&kenlex_wd, thread_name);

    if (!kenlex_thread_list[kenlex_wd]) {
        printk("Error listening to kenlex wd %d: Error %d", kenles_wd, errno);
        return -1;
    }

    return 0;
}

int stop_listening(int kenlex_wd) {
    if (kthread_stop(kenlex_thread_list[kenlex_wd]) != 0) {
        printk ("Error stopping thread with kenlex wd %d: Error %d\n", kenlex_wd, errno);
        return -1;
    }
    return 0;
}

void listen(int wd) {
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(struct pollfd));
    
    pfd.fd = inotify_fd;
    pfd.events = 0xffff;

    struct pollfd pfd_array[1];
    pfd_array[0] = pfd;

    int buffer_len = 15 * sizeof(struct inotify_event);
    char buffer[buffer_len] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t len, i = 0;


    while (1) {
        int poll_rc = poll(pfd_array, 1, -1);
        if (poll_rc < 0) {
            printk("Error polling event: Error %d\n", errno);
            return;
        }

        if (pfd_array[0].revents & POLLIN) {
            len = read(pfd_array[0].fd, buffer, buffer_len);
            if (len < 0) {
                printk ("Error reading event: Error %d\n", errno);
                return;
            }

            while (i < len) {
                struct inotify_event* event = (struct inotify_event*) &buffer[i];
                
                if (event -> wd == wd) {
                    // TODO populate some kenlex event struct and write it to VFS
                }

                i += sizeof(struct inotify_event) + event -> len;
            }

            i = 0;
            memset(buffer, 0, buffer_len);
        }
    }
}

int kenlex_cleanup() {
    free(inotify_wd);
    return 0;
}

int kenlex_add_path(const char* path) {

    if (inotify_fd > 0) {
        // All events for now       
        int wd = inotify_add_watch(inotify_fd, path, IN_ALL_EVENTS);

        if (wd < 0) {
            printk("Error adding path to inotify: Error %d\n", errno)
            return -1;
        }
       
        if (wd_size == wd_max_size) {

            wd_max_size += 10;
            int* new_wd = (int*)malloc(wd_max_size * sizeof(int))
            struct task_struct** new_thread_list = (struct task_struct**)malloc(wd_max_size * sizeof(task_struct*))

            for (int i = 0; i < wd_size; i++) {
                new_wd[i] = inotify_wd[i];
                new_thread_list[i] = kenlex_thread_list[i];
            }

            free(inotify_wd);
            free(kenlex_thread_list);

            inotify_wd = new_wd;
            kenlex_thread_list = new_thread_list;
        }

        inotify_wd[wd_size] = wd;
        wd_size += 1;
        return wd_size;
    }

    printk("Kenlex monitor not setup yet (Have you called setup_kenlex_monitor()?)\n");
    return -1;
}

