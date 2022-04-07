#include "kenlex_structures.h"
#include "kenlex_notifications.h"
#include "kenlex_processing.h"
#include <sys/inotify.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


void* event_processing_thread(void* arg) {
    struct events_queue_struct event;
    struct item_settings_struct item_settings;

    while (1) {

        if(dequeue_event(&event) == 0) {
            //printf("mask: %d item: %s item_len: %d\n", event.event_mask, event.item, event.item_len);

            char* item = event.item;
            int mask = event.event_mask;
            int read = 0, write = 0, access = 0;

            if (mask & IN_ACCESS || mask & IN_CLOSE_NOWRITE || mask & IN_OPEN) {
                read = 1;
            }

            if (mask & IN_ATTRIB || mask & IN_CLOSE_WRITE || mask & IN_MODIFY) {
                write = 1;
            }

            if (mask & IN_CREATE || mask & IN_DELETE || mask & IN_DELETE_SELF || mask & IN_MOVE_SELF || mask & IN_MOVED_FROM || mask & IN_MOVED_FROM) {
                access = 1;
            }

            int item_read_severity = 0;
            int item_write_severity = 0;
            int item_access_severity = 0;            
            int log_severity_threshold = get_log_severity();
            int email_severity_threshold = get_email_severity();

            if (get_item_settings(item, &item_settings) == 0) {
                item_read_severity = item_settings.reads.severity;
                item_write_severity = item_settings.writes.severity;
                item_access_severity = item_settings.accesses.severity;
            }

            if (read) {
                if (item_read_severity >= log_severity_threshold) {  
                    kenlex_log_event(item, mask, item_read_severity);
                }

                if (item_read_severity >= email_severity_threshold) {
                    // Email it
                }
            }

            if (write) {
                if (item_write_severity >= log_severity_threshold) {
                    kenlex_log_event(item, mask, item_write_severity);
                }

                if (item_write_severity >= email_severity_threshold) {
                    // Email it
                }
            }

            if (access) {
                if (item_access_severity >= log_severity_threshold) {
                    kenlex_log_event(item, mask, item_access_severity);
                }

                if (item_access_severity >= email_severity_threshold) {
                    // Email it
                }
            }
        }
    }
}

int begin_event_processing(pthread_t* processing_thread) {
    int rc = -1;
    static int thread_started = 0;
    if (!thread_started) {        
        pthread_create(processing_thread, NULL, event_processing_thread, "processing_thread");
        thread_started = 1;
        rc = 0;
    }
    return rc;
}