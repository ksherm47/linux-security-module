#include "kenlex_structures.h"

void add_event_to_queue(char* item, int item_len, int event_mask) {
    pthread_mutex_lock(&events_queue_mutex);
    if(!events_queue) {
        events_queue = (struct events_queue_struct*)malloc(sizeof(struct events_queue_struct));
        events_queue -> event_mask = event_mask;
        events_queue -> item = item;
        events_queue -> item_len = item_len;
        events_queue -> next = 0;
        events_queue -> prev = 0;
        events_queue_end = events_queue;
    } else {
        struct events_queue_struct* new_event = (struct events_queue_struct*)malloc(sizeof(struct events_queue_struct));
        new_event -> event_mask = event_mask;
        new_event -> item = item;
        new_event -> item_len = item_len;

        new_event -> next = events_queue;
        new_event -> prev = 0;
        events_queue -> prev = new_event;
        events_queue = new_event;
    }
    num_events++;
    pthread_mutex_unlock(&events_queue_mutex);
}

int dequeue_event(struct events_queue_struct* event) {
    pthread_mutex_lock(&events_queue_mutex);
    int ret = -1;
    if (events_queue != NULL) {
        event -> event_mask = events_queue_end -> event_mask;
        event -> item = events_queue_end -> item;
        event -> item_len = events_queue_end -> item_len;

        if (events_queue_end -> prev != 0) {
            events_queue_end = events_queue_end -> prev;
            free(events_queue_end -> next);
            events_queue_end -> next = 0;
        } else {
            free(events_queue_end);
            events_queue = 0;
            events_queue_end = 0;
        } 
        ret = 0;
        num_events--;
    }
    pthread_mutex_unlock(&events_queue_mutex);
    return ret;
}