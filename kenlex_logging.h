#ifndef KENLEX_LOGGING_H
#define KENLEX_LOGGING_H
#include "kenlex_structures.h"

static int log_fd;
int kenlex_log_init(char* log_filename);
int kenlex_log(char* item, int mask, int severity);

#endif