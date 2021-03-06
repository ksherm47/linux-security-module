#ifndef KENLEX_NOTIFICATIONS_H
#define KENLEX_NOTIFICATIONS_H

static int log_fd = -1;
static char* log_file = 0;
int kenlex_log_init(char* log_filename);
int kenlex_log_event(char* item, int mask, int severity);
int kenlex_log_error(char* error_msg);
int kenlex_email_event(char* item, int mask, int severity, char** email_addresses, int num_email_addresses, int event_type);
int kenlex_log_frequency(char* item, int frequency, long int time_frame, int event_type);
int kenlex_email_frequency(char* item, int frequency, long int time_frame, char** email_addresses, int num_email_addresses, int event_type);

#endif