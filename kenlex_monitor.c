#include <sys/inotify.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>
#include "kenlex_structures.h"

static int inotify_fd = -1;
static int* inotify_wd;
static int* active_wd = NULL;
static char** wd_item_names = NULL;
static int num_active_wd = 0;
static int max_active_wd = 10;
static int wd_size = 0;
static int wd_max_size = 10;

pthread_mutex_t active_wd_mutex;

void* inotify_listen(void* arg);

int setup_kenlex_monitor(pthread_t* monitor_thread) {
    inotify_fd = inotify_init1(0);
    if (inotify_fd < 0) {
        printf("Error opening inotify instance: Error number %d\n", errno);
        return -1;
    }

    inotify_wd = (int*)malloc(wd_max_size * sizeof(int));
    active_wd = (int*)malloc(max_active_wd * sizeof(int));
    wd_item_names = (char**)malloc(max_active_wd * sizeof(char*));

    int rc = pthread_create(monitor_thread, NULL, inotify_listen, "monitor_thread");
    if (rc < 0) {
        printf("Error starting inotify thread: Error number %d\n", errno);
        return -1;
    }

    return 0;
}

void* inotify_listen(void* arg) {
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
            printf("Error polling event: Error number %d\n", errno);
            return (void*)-1;
        }

        if (pfd_array[0].revents & POLLIN) {
            len = read(pfd_array[0].fd, buffer, buffer_len);
            if (len < 0) {
                printf("Error reading event: Error number %d\n", errno);
                return (void*)-1;
            }

            while (i < len) {
                struct inotify_event* event = (struct inotify_event*) &buffer[i];
                
                pthread_mutex_lock(&active_wd_mutex);
                for (j = 0; j < num_active_wd; j++) {                  
                    if (event -> wd == inotify_wd[active_wd[j]]) {
                        char* item = wd_item_names[active_wd[j]];
                        int item_len = strlen(wd_item_names[active_wd[j]]);

                        add_event_to_queue(item, item_len, event -> mask);
                        break;
                    }
                }
                pthread_mutex_unlock(&active_wd_mutex);

                i += sizeof(struct inotify_event) + event -> len;
            }

            i = 0;
            memset(buffer, 0, buffer_len);
        }
    }
}

int kenlex_add_path(const char* path) {
    int i, wd;
    int* new_wd;
    char** new_wd_names;

    if (inotify_fd > 0) {

        char full_path[PATH_MAX];
        realpath(path, full_path);
        
        // All events for now       
        wd = inotify_add_watch(inotify_fd, full_path, IN_ALL_EVENTS);

        if (wd < 0) {
            printf("Error adding path to inotify: Error number %d\n", errno);
            return -1;
        }
       
        if (wd_size == wd_max_size) {

            wd_max_size += 10;
            new_wd = (int*)malloc(wd_max_size * sizeof(int));
            new_wd_names = (char**)malloc(wd_max_size * sizeof(char*));

            for (i = 0; i < wd_size; i++) {
                new_wd[i] = inotify_wd[i];
                new_wd_names[i] = (char*)malloc(strlen(wd_item_names[i]) + 1);
                memcpy(new_wd_names[i], wd_item_names[i], strlen(wd_item_names[i]) + 1);
            }

            free(inotify_wd);

            for (int i = 0; i < wd_size; i++) {
                free(wd_item_names[i]);
            }
            free(wd_item_names);

            inotify_wd = new_wd;
            wd_item_names = new_wd_names;
        }

        struct event_settings_struct settings;
        settings.severity = 0;
        settings.frequency = -1;
        settings.time_frame = PER_SECOND;
        add_item_setting((char*)full_path, strlen(full_path), settings, READ | WRITE | ACCESS);

        inotify_wd[wd_size] = wd;
        wd_item_names[wd_size] = (char*)malloc(strlen(full_path) + 1);
        memcpy(wd_item_names[wd_size], full_path, strlen(full_path));

        wd_size += 1;
        return wd_size - 1;
    }

    printf("Kenlex monitor not setup yet (Have you called setup_kenlex_monitor()?)\n");
    return -1;
}

int listen_for_kenlex_events(int kenlex_wd) {
    int i;
    int* new_active_wd;
    
    pthread_mutex_lock(&active_wd_mutex);
    if (num_active_wd == max_active_wd) {
        max_active_wd += 10;
        new_active_wd = (int*)malloc(max_active_wd * sizeof(int));

        for (i = 0; i < num_active_wd; i++) {
            new_active_wd[i] = active_wd[i];
        }

        free(active_wd);

        active_wd = new_active_wd;
    }

    active_wd[num_active_wd] = kenlex_wd;
    num_active_wd += 1;
    pthread_mutex_unlock(&active_wd_mutex);
    return 0;
}

int stop_listening(int kenlex_wd) {  
    int i;
    pthread_mutex_lock(&active_wd_mutex);
    for (i = 0; i < num_active_wd; i++) {
        if (kenlex_wd == active_wd[i]) {
            active_wd[i] = -1;
        }
    }
    pthread_mutex_unlock(&active_wd_mutex);
    return 0;
}

int kenlex_cleanup(void) {
    free(inotify_wd);
    free(active_wd);
    return 0;
}



