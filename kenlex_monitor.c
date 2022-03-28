#include <linux/inotify.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/slab.h>

static int inotify_fd = -1;
static int* inotify_wd;
static int* active_wd = NULL;
static int num_active_wd = 0;
static int max_active_wd = 10;
static int wd_size = 0;
static int wd_max_size = 10;

static struct task_struct* inotify_thread;
struct mutex active_wd_mutex;

int inotify_listen(void* data);

int setup_kenlex_monitor(void) {
    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        printk("Error opening inotify instance\n");
        return -1;
    }

    inotify_wd = (int*)kmalloc(wd_max_size * sizeof(int), 0);
    active_wd = (int*)kmalloc(max_active_wd * sizeof(int), 0);

    inotify_thread = kthread_create(&inotify_listen, NULL, "inotify_thread");
    if (!inotify_thread) {
        printk("Error starting inotify thread\n");
        return -1;
    }

    return 0;
}

int inotify_listen(void* data) {
    struct pollfd pfd;
    struct pollfd pfd_array[1];
    int buffer_len;

    memset(&pfd, 0, sizeof(struct pollfd));
    
    pfd.fd = inotify_fd;
    pfd.events = 0xffff;

    pfd_array[0] = pfd;

    buffer_len = 15 * sizeof(struct inotify_event);
    char buffer[buffer_len] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t len, i = 0, j;


    while (1) {
        int poll_rc = poll(pfd_array, 1, -1);
        if (poll_rc < 0) {
            printk("Error polling event\n");
            return - 1;
        }

        if (pfd_array[0].revents & POLLIN) {
            len = read(pfd_array[0].fd, buffer, buffer_len);
            if (len < 0) {
                printk ("Error reading event\n");
                return -1;
            }

            while (i < len) {
                struct inotify_event* event = (struct inotify_event*) &buffer[i];
                
                mutex_lock(&active_wd_mutex);
                for (j = 0; j < num_active_wd; j++) {                  
                    if (event -> wd == inotify_wd[active_wd[i]]) {
                        // TODO populate some kenlex event struct and write it to VFS
                        break;
                    }
                }
                mutex_unlock(&active_wd_mutex);

                i += sizeof(struct inotify_event) + event -> len;
            }

            i = 0;
            memset(buffer, 0, buffer_len);
        }
    }
    return 0;
}

int kenlex_add_path(const char* path) {
    int i, wd;
    int* new_wd;

    if (inotify_fd > 0) {
        // All events for now       
        wd = inotify_add_watch(inotify_fd, path, IN_ALL_EVENTS);

        if (wd < 0) {
            printk("Error adding path to inotify\n");
            return -1;
        }
       
        if (wd_size == wd_max_size) {

            wd_max_size += 10;
            new_wd = (int*)kmalloc(wd_max_size * sizeof(int), 0);

            for (i = 0; i < wd_size; i++) {
                new_wd[i] = inotify_wd[i];
            }

            kfree(inotify_wd);

            inotify_wd = new_wd;
        }

        inotify_wd[wd_size] = wd;
        wd_size += 1;
        return wd_size - 1;
    }

    printk("Kenlex monitor not setup yet (Have you called setup_kenlex_monitor()?)\n");
    return -1;
}

int listen_for_kenlex_events(int kenlex_wd) {
    int i;
    int* new_active_wd;
    
    mutex_lock(&active_wd_mutex);
    if (num_active_wd == max_active_wd) {
        max_active_wd += 10;
        new_active_wd = (int*)kmalloc(max_active_wd * sizeof(int), 0);

        for (i = 0; i < num_active_wd; i++) {
            new_active_wd[i] = active_wd[i];
        }

        kfree(active_wd);

        active_wd = new_active_wd;
    }

    num_active_wd += 1;
    active_wd[num_active_wd] = kenlex_wd;
    mutex_unlock(&active_wd_mutex);
    return 0;
}

int stop_listening(int kenlex_wd) {  
    int i;
    mutex_lock(&active_wd_mutex);
    for (i = 0; i < num_active_wd; i++) {
        if (kenlex_wd == active_wd[i]) {
            active_wd[i] = -1;
        }
    }
    mutex_unlock(&active_wd_mutex);
    return 0;
}

int kenlex_cleanup(void) {
    kfree(inotify_wd);
    kfree(active_wd);
    return 0;
}



