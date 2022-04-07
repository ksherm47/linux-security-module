#include "kenlex_structures.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>

void add_event_to_queue(char* item, int item_len, int event_mask, long int timestamp) {
    pthread_mutex_lock(&events_queue_mutex);
    if(!events_queue_tail) {
        events_queue_head = (struct events_queue_struct*)malloc(sizeof(struct events_queue_struct));
        events_queue_head -> event_mask = event_mask;
        events_queue_head -> item = item;
        events_queue_head -> item_len = item_len;
        events_queue_head -> timestamp = timestamp;
        events_queue_head -> next = 0;
        events_queue_head -> prev = 0;
        events_queue_tail = events_queue_head;
    } else {
        struct events_queue_struct* new_event = (struct events_queue_struct*)malloc(sizeof(struct events_queue_struct));
        new_event -> event_mask = event_mask;
        new_event -> item = item;
        new_event -> item_len = item_len;
        new_event -> timestamp = timestamp;

        new_event -> next = events_queue_head;
        new_event -> prev = 0;
        events_queue_head -> prev = new_event;
        events_queue_head = new_event;
    }
    num_events++;
    pthread_mutex_unlock(&events_queue_mutex);
}

int dequeue_event(struct events_queue_struct* event) {
    pthread_mutex_lock(&events_queue_mutex);
    int ret = -1;
    if (events_queue_head != NULL) {
        event -> event_mask = events_queue_tail -> event_mask;
        event -> item = events_queue_tail -> item;
        event -> item_len = events_queue_tail -> item_len;
        event -> timestamp = events_queue_tail -> timestamp;

        if (events_queue_tail -> prev != 0) {
            events_queue_tail = events_queue_tail -> prev;
            free(events_queue_tail -> next);
            events_queue_tail -> next = 0;
        } else {
            free(events_queue_tail);
            events_queue_head = 0;
            events_queue_tail = 0;
        } 
        ret = 0;
        num_events--;
    }
    pthread_mutex_unlock(&events_queue_mutex);
    return ret;
}

void add_item_setting(char* item, int item_len, struct event_settings_struct event_settings, int setting_type) {
    pthread_mutex_lock(&item_settings_list_mutex);

    char full_path[PATH_MAX];
    realpath(item, full_path);

    if (!item_settings_list) {
        item_settings_list = (struct item_settings_list_struct*)malloc(sizeof(struct item_settings_list_struct));
        item_settings_list -> item = (char*)malloc(strlen(full_path) + 1);
        memcpy(item_settings_list -> item, full_path, strlen(full_path) + 1);
        item_settings_list -> item_len = item_len;

        if (setting_type & READ) {
            (item_settings_list -> item_settings).reads = event_settings;
        }

        if (setting_type & WRITE) {
            (item_settings_list -> item_settings).writes = event_settings;
        }

        if (setting_type & ACCESS) {
            (item_settings_list -> item_settings).accesses = event_settings;
        }

        (item_settings_list -> item_settings).num_read_events = 0;
        (item_settings_list -> item_settings).num_write_events = 0;
        (item_settings_list -> item_settings).num_access_events = 0;
        (item_settings_list -> item_settings).last_alert = -1;
        item_settings_list -> next = 0;

    } else {
        struct item_settings_list_struct* runner = item_settings_list;

        while (strcmp(runner -> item, full_path) && runner -> next != 0) {
            runner = runner -> next;
        }

        if (!strcmp(runner -> item, full_path)) {

            if (setting_type & READ) {
                (runner -> item_settings).reads = event_settings;
            }

            if (setting_type & WRITE) {
                (runner -> item_settings).writes = event_settings;
            }

            if (setting_type & ACCESS) {
                (runner -> item_settings).accesses = event_settings;
            }

        } else {
            struct item_settings_list_struct* new_item_setting = (struct item_settings_list_struct*)malloc(sizeof(struct item_settings_list_struct));
            new_item_setting -> item = (char*)malloc(strlen(full_path) + 1);
            memcpy(new_item_setting -> item, full_path, strlen(full_path) + 1);
            new_item_setting -> item_len = item_len;

            if (setting_type & READ) {
                (new_item_setting -> item_settings).reads = event_settings;
            }

            if (setting_type & WRITE) {
                (new_item_setting -> item_settings).writes = event_settings;
            }

            if (setting_type & ACCESS) {
                (new_item_setting -> item_settings).accesses = event_settings;
            }
            
            (item_settings_list -> item_settings).num_read_events = 0;
            (item_settings_list -> item_settings).num_write_events = 0;
            (item_settings_list -> item_settings).num_access_events = 0;
            (item_settings_list -> item_settings).last_alert = -1;
            new_item_setting -> next = 0;
            runner -> next = new_item_setting;
        }
    }
    
    pthread_mutex_unlock(&item_settings_list_mutex);
}

int get_item_settings(char* item, struct item_settings_struct* item_settings) {
    pthread_mutex_lock(&item_settings_list_mutex);

    char full_path[PATH_MAX];
    realpath(item, full_path);

    int rc = -1;
    if (item_settings_list) {
        struct item_settings_list_struct* runner = item_settings_list;

        while(strcmp(runner -> item, full_path) && runner -> next != 0) {
            runner = runner -> next;
        }

        if (!strcmp(runner -> item, full_path)) {
            item_settings -> reads = (runner -> item_settings).reads;
            item_settings -> writes = (runner -> item_settings).writes;
            item_settings -> accesses = (runner -> item_settings).accesses;
            item_settings -> last_alert = (runner -> item_settings).last_alert;
            item_settings -> num_read_events = (runner -> item_settings).num_read_events;
            item_settings -> num_write_events = (runner -> item_settings).num_write_events;
            item_settings -> num_access_events = (runner -> item_settings).num_access_events;
            rc = 0;
        }
    }

    pthread_mutex_unlock(&item_settings_list_mutex);
    return rc;
}

int update_last_alert(char* item, long last_alert) {
    pthread_mutex_lock(&item_settings_list_mutex);

    char full_path[PATH_MAX];
    realpath(item, full_path);

    int rc = -1;
    if(item_settings_list) {
        struct item_settings_list_struct* runner = item_settings_list;

        while(strcmp(runner -> item, full_path) && runner -> next != 0) {
            runner = runner -> next;
        }

        if (!strcmp(runner -> item, full_path)) {
            (runner -> item_settings).last_alert = last_alert;
            rc = 0;
        }
    }

    pthread_mutex_unlock(&item_settings_list_mutex);
    return rc;
}

int update_num_events(char* item, int num_events, int type) {
    pthread_mutex_lock(&item_settings_list_mutex);

    char full_path[PATH_MAX];
    realpath(item, full_path);

    int rc = -1;
    if(item_settings_list) {
        struct item_settings_list_struct* runner = item_settings_list;

        while(strcmp(runner -> item, full_path) && runner -> next != 0) {
            runner = runner -> next;
        }

        if (!strcmp(runner -> item, full_path)) {
            if (type & READ) {        
                (runner -> item_settings).num_read_events = num_events;
            }
            if (type & WRITE) {        
                (runner -> item_settings).num_write_events = num_events;
            }
            if (type & ACCESS) {        
                (runner -> item_settings).num_access_events = num_events;
            }
            rc = 0;
        }
    }

    pthread_mutex_unlock(&item_settings_list_mutex);
    return rc;
}

void set_log_severity(int severity) {
    pthread_mutex_lock(&global_settings_mutex);
    global_settings.log_severity = severity;
    pthread_mutex_unlock(&global_settings_mutex);
}

int get_log_severity() {
    pthread_mutex_lock(&global_settings_mutex);
    int severity = global_settings.log_severity;
    pthread_mutex_unlock(&global_settings_mutex);
    return severity;
}

void set_email_severity(int severity) {
    pthread_mutex_lock(&global_settings_mutex);
    global_settings.email_severity = severity;
    pthread_mutex_unlock(&global_settings_mutex);
}

int get_email_severity() {
    pthread_mutex_lock(&global_settings_mutex);
    int severity = global_settings.email_severity;
    pthread_mutex_unlock(&global_settings_mutex);
    return severity;
}