#ifndef KENLEX_MONITOR_H
#define KENLEX_MONITOR_H

#include <pthread.h>

int setup_kenlex_monitor(pthread_t* monitor_thread);
int kenlex_cleanup();
int kenlex_add_path(const char* path);
int listen_for_kenlex_events(int kenlex_wd);
int stop_listening(int kenlex_wd);

#endif