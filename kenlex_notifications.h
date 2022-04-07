#ifndef KENLEX_NOTIFICATIONS_H
#define KENLEX_NOTIFICATIONS_H

static int log_fd = -1;
int kenlex_log_init(char* log_filename);
int kenlex_log_event(char* item, int mask, int severity);
int kenlex_email_event(char* item, int mask, int severity);
int kenlex_log_frequency(char* item, int frequency, int time_frame);
int kenlex_email_frequency(char* item, int frequency, int time_frame);

#endif