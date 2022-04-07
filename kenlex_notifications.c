#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>
#include "kenlex_notifications.h"
#include "kenlex_structures.h"

int kenlex_log_init(char* log_filename) {
    log_fd = open(log_filename, O_RDWR | O_CREAT | O_APPEND, 0644);

    if (log_fd < 0) {
        return -1;
    }
}

int kenlex_log_error(char* error_msg) {
    int rc = -1;
    if (log_fd > 0) {

        time_t temp = time(NULL);
        struct tm* timeptr = localtime(&temp);
        char date_string[100];

        strftime(date_string, 100, "%D %r", timeptr);

        char error_message[512];
        snprintf(error_message, 512, "(%s) ERROR: %s\n", date_string, error_message);

        write(log_fd, error_message, 512);
        rc = 0;   
    }
    return rc;
}

int kenlex_log_event(char* item, int mask, int severity) {
    int rc = -1;
    if (log_fd > 0) {

        char* event_description = 0;

        if (mask & IN_ACCESS) {
            event_description = "File was accessed";
        }

        else if (mask & IN_ATTRIB) {
            event_description = "File metadata changed";
        }

        else if (mask & IN_CLOSE_WRITE) {
            event_description = "File opened for writing was closed";
        }

        else if (mask & IN_CLOSE_NOWRITE) {
            event_description = "File or directory not opened for writing was closed";
        }

        else if (mask & IN_CREATE) {
            event_description = "File/directory created in watched directory";
        }

        else if (mask & IN_DELETE) {
            event_description = "File/directory deleted from watched directory";
        }

        else if (mask & IN_DELETE_SELF) {
            event_description = "File/directory deleted";
        }

        else if (mask & IN_MODIFY) {
            event_description = "File was modified";
        }

        else if (mask & IN_MOVE_SELF) {
            event_description = "File/directory was moved";
        }

        else if (mask & IN_MOVED_FROM) {
            event_description = "File moved out of directory";
        }

        else if (mask & IN_MOVED_TO) {
            event_description = "File moved into directory";
        }

        else if (mask & IN_OPEN) {
            event_description = "File or directory was opened";
        }

        time_t temp = time(NULL);
        struct tm* timeptr = localtime(&temp);
        char date_string[100];

        strftime(date_string, 100, "%D %r", timeptr);
        
        char log_message[512];
        snprintf(log_message, 512, "(%s) %s: %s (Severity %d)\n", date_string, item, event_description, severity);

        write(log_fd, log_message, strlen(log_message));
        if (event_description) {
            event_description = 0;
        }
        rc = 0;
    }
    return rc;
}

int kenlex_email_event(char* item, int mask, int severity) {
    return -1;
}

int kenlex_log_frequency(char* item, int frequency, long int time_frame, int event_type) {
    int rc = -1;

    if (log_fd > 0) {
        time_t temp = time(NULL);
        struct tm* timeptr = localtime(&temp);
        char date_string[100];

        strftime(date_string, 100, "%D %r", timeptr);

        char* event_type_str;
        switch(event_type) {
            case READ:
                event_type_str = "read";
                break;
            case WRITE:
                event_type_str = "write";
                break;
            case ACCESS:
                event_type_str = "access";
                break;
            default:
                event_type_str = "";
                break;
        }

        char log_message[512];
        snprintf(log_message, 512, "(%s) %s: %d %s events triggered within %d milliseconds\n", date_string, item, frequency, event_type_str, time_frame);

        write(log_fd, log_message, strlen(log_message));
        rc = 0;
    }
    return rc;
}

int kenlex_email_frequency(char* item, int frequency, long int time_frame) {
    return -1;
}