#include "kenlex_structures.h"

void add_event_to_queue(char* item, int item_len, int event_mask) {
    pthread_mutex_lock(&events_queue_mutex);
    if(!events_queue_tail) {
        events_queue_head = (struct events_queue_struct*)malloc(sizeof(struct events_queue_struct));
        events_queue_head -> event_mask = event_mask;
        events_queue_head -> item = item;
        events_queue_head -> item_len = item_len;
        events_queue_head -> next = 0;
        events_queue_head -> prev = 0;
        events_queue_tail = events_queue_head;
    } else {
        struct events_queue_struct* new_event = (struct events_queue_struct*)malloc(sizeof(struct events_queue_struct));
        new_event -> event_mask = event_mask;
        new_event -> item = item;
        new_event -> item_len = item_len;

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