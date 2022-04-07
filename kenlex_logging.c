#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <time.h>
#include "kenlex_logging.h"

int kenlex_log_init(char* log_filename) {
    log_fd = open(log_filename, O_RDWR | O_CREAT | O_APPEND, 0644);

    if (log_fd < 0) {
        return -1;
    }
}

int kenlex_log_event(char* item, int mask, int severity) {

    if (log_fd > 0) {

        char* event_description = 0;;
        int len;

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
        
        char log_message[256];
        snprintf(log_message, 256, "(%s) %s: %s (Severity %d)\n", date_string, item, event_description, severity);

        write(log_fd, log_message, strlen(log_message));
        if (event_description) {
            event_description = 0;
        }
    }
}