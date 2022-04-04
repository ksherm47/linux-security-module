#ifndef KENLEX_STRUCTURES_H
#define KENLEX_STRUCTURES_H
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t events_queue_mutex;
static pthread_mutex_t settings_list_mutex;

struct events_queue_struct {
    struct events_queue_struct* next;
    struct events_queue_struct* prev;
    int event_mask;
    char* item;
    int item_len;
};

struct settings_list_struct {
    struct settings_list_struct* next;
    int id;
};

static struct events_queue_struct* events_queue = 0;
static struct settings_list_struct* settings_list = 0;
static struct events_queue_struct* events_queue_end = 0;
static struct settings_list_struct* settings_list_end = 0;
static int num_events = 0;
static int settings_size = 0;

void add_event_to_queue(char* item, int item_len, int event_mask);
int dequeue_event(struct events_queue_struct* event);

#endif