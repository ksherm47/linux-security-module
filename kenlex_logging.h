#ifndef KENLEX_LOGGING_H
#define KENLEX_LOGGING_H
#include "kenlex_structures.h"

static int log_fd;
int init_log(char* log_filename);
int log(char* item, int mask, int severity);

#endif