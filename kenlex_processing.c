#include "kenlex_structures.h"
#include "kenlex_notifications.h"
#include "kenlex_processing.h"
#include <sys/inotify.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define ONE_SECOND 1e9
#define ONE_MINUTE 60e9
#define ONE_HOUR 36e11
#define ONE_DAY 864e11


void* event_processing_thread(void* arg) {
    struct events_queue_struct event;
    struct item_settings_struct item_settings;

    while (1) {

        if(dequeue_event(&event) == 0) {
            //printf("mask: %d item: %s item_len: %d\n", event.event_mask, event.item, event.item_len);

            char* item = event.item;
            int mask = event.event_mask;
            long int timestamp = event.timestamp;
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

            int item_read_severity, item_read_frequency, item_read_time_frame;
            int item_write_severity, item_write_frequency, item_write_time_frame;
            int item_access_severity, item_access_frequency, item_access_time_frame;
            time_t last_alert;
            int num_read_events, num_write_events, num_access_events;

            int log_severity_threshold = get_log_severity();
            int email_severity_threshold = get_email_severity();
            char** email_addresses = get_email_addresses();
            int num_email_addresses = get_num_email_addresses();

            if (get_item_settings(item, &item_settings) == 0) {
                item_read_severity = item_settings.reads.severity;
                item_read_frequency = item_settings.reads.frequency;
                item_read_time_frame = item_settings.reads.time_frame;

                item_write_severity = item_settings.writes.severity;
                item_write_frequency = item_settings.writes.frequency;
                item_write_time_frame = item_settings.writes.time_frame;

                item_access_severity = item_settings.accesses.severity;
                item_access_frequency = item_settings.accesses.frequency;
                item_access_time_frame = item_settings.accesses.time_frame;

                last_alert = item_settings.last_alert;
                num_read_events = item_settings.num_read_events;
                num_write_events = item_settings.num_write_events;
                num_access_events = item_settings.num_access_events;

            } else {
                char error_msg[512];
                snprintf(error_msg, 512, "Settings not found for item %s", item);
                kenlex_log_error(error_msg);
                continue;
            }

            if (read) {
                num_read_events++;
                update_num_events(item, num_read_events, READ);

                if (item_read_severity >= log_severity_threshold) {  
                    kenlex_log_event(item, mask, item_read_severity);
                }

                if (item_read_severity >= email_severity_threshold) {
                    kenlex_email_event(item, mask, item_read_severity, email_addresses, num_email_addresses, READ);
                }

                if (item_read_frequency > 0) {
                    
                    if (last_alert == -1) {
                        last_alert = timestamp;
                        update_last_alert(item, timestamp);
                    }
                    
                    if (timestamp - last_alert <= item_read_time_frame) {                    
                        if (num_read_events == item_read_frequency) {
                            kenlex_log_frequency(item, item_read_frequency, item_read_time_frame, READ);
                            kenlex_email_frequency(item, item_read_frequency, item_read_time_frame, email_addresses, num_email_addresses, READ);

                            update_last_alert(item, timestamp);
                            update_num_events(item, 0, READ);
                        }
                    } else {
                        update_last_alert(item, timestamp);
                        update_num_events(item, 1, READ);
                    }

                }
            }

            if (write) {
                num_write_events++;
                update_num_events(item, num_write_events, WRITE);

                if (item_write_severity >= log_severity_threshold) {
                    kenlex_log_event(item, mask, item_write_severity);
                }

                if (item_write_severity >= email_severity_threshold) {
                    kenlex_email_event(item, mask, item_write_severity, email_addresses, num_email_addresses, WRITE);
                }

                if (item_write_frequency > 0) {
                    
                    if (last_alert == -1) {
                        last_alert = timestamp;
                        update_last_alert(item, timestamp);
                    }
                    
                    if (timestamp - last_alert <= item_write_time_frame) {
                        if (num_write_events == item_write_frequency) {
                            kenlex_log_frequency(item, item_write_frequency, item_write_time_frame, WRITE);
                            kenlex_email_frequency(item, item_write_frequency, item_write_time_frame, email_addresses, num_email_addresses, WRITE);

                            update_last_alert(item, timestamp);
                            update_num_events(item, 0, WRITE);
                        }
                    } else {
                        update_last_alert(item, timestamp);
                        update_num_events(item, 1, WRITE);
                    }

                }
            }

            if (access) {
                num_access_events++;
                update_num_events(item, num_read_events, ACCESS);

                if (item_access_severity >= log_severity_threshold) {
                    kenlex_log_event(item, mask, item_access_severity);
                }

                if (item_access_severity >= email_severity_threshold) {
                    kenlex_email_event(item, mask, item_access_severity, email_addresses, num_email_addresses, ACCESS);
                }

                if (item_access_frequency > 0) {
                    
                    if (last_alert == -1) {
                        last_alert = timestamp;
                        update_last_alert(item, timestamp);
                    }
                    

                    if (timestamp - last_alert <= item_access_time_frame) {               
                        if (num_access_events == item_access_frequency) {
                            kenlex_log_frequency(item, item_access_frequency, item_access_time_frame, ACCESS);
                            kenlex_email_frequency(item, item_access_frequency, item_access_time_frame, email_addresses, num_email_addresses, ACCESS);

                            update_last_alert(item, timestamp);
                            update_num_events(item, 0, ACCESS);
                        }
                    } else {
                        update_last_alert(item, timestamp);
                        update_num_events(item, 1, ACCESS);
                    }           
                    
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