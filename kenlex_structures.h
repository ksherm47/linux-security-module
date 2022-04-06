#ifndef KENLEX_STRUCTURES_H
#define KENLEX_STRUCTURES_H
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define PER_SECOND 0
#define PER_MINUTE 1
#define PER_HOUR 2
#define PER_DAY 3

#define READ 0x00000001
#define WRITE 0x00000010
#define ACCESS 0x00000100

static pthread_mutex_t events_queue_mutex;
static pthread_mutex_t item_settings_list_mutex;
static pthread_mutex_t global_settings_mutex;

struct events_queue_struct {
    struct events_queue_struct* next;
    struct events_queue_struct* prev;
    int event_mask;
    char* item;
    int item_len;
};

struct event_settings_struct {
    int severity;
    int frequency;
    int time_frame;
};

struct item_settings_struct {
    struct event_settings_struct reads;
    struct event_settings_struct writes;
    struct event_settings_struct accesses;
}

struct item_settings_list_struct {
    struct item_settings_list_struct* next;
    char* item;
    int item_len;
    struct item_settings_struct item_settings;
};

struct global_settings_struct {
    int log_severity;
    int email_severity;
    char** email_addresses;
    int num_email_addresses;
};

static struct events_queue_struct* events_queue_head = 0;
static struct item_settings_list_struct* item_settings_list = 0;
static struct events_queue_struct* events_queue_tail = 0;
static struct item_settings_list_struct* settings_list_end = 0;
static int num_events = 0;
static int settings_size = 0;

static struct global_settings_struct global_settings;
global_settings.log_severity = 0;
global_settings.email_severity = 0;
global_settings.email_addresses = 0;
global_settings.num_email_addresses = 0;

void add_event_to_queue(char* item, int item_len, int event_mask);
int dequeue_event(struct events_queue_struct* event);

void add_item_setting(char* item, int item_len, struct event_settings_struct event_settings, int setting_type);
int get_item_settings(char* item, struct item_settings_struct* item_settings); 
//void remove_item_setting(char* item);

void update_log_severity(int severity);
int get_log_severity();
void update_email_severity(int severity);
int get_email_severity();
void add_email_address(char* email);

#endif