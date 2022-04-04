#include "kenlex_structures.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


void* event_processing_thread(void* arg) {
    struct events_queue_struct event;
    while (1) {
        if(dequeue_event(&event) == 0) {
            printf("mask: %d item: %s item_len: %d\n", event.event_mask, event.item, event.item_len);
        }
    }
}

void begin_event_processing() {
    pthread_t processing_thread;
    pthread_create(&processing_thread, NULL, event_processing_thread, "processing thread");
    while(1) {
    }
}