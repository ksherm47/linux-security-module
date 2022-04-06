#include <fcntl.h>
#include <stdio.h>
#include "kenlex_logging.h"

int init_log(char* log_filename) {
    log_fd = open(log_filename, O_RDWR | O_CREAT, 0644);

    if (log_fd < 0) {
        return -1;
    }
}

int log(char* item, int mask, int severity) {

    if (log_fd > 0) {

        char* event_description;

        if (mask & IN_ACCESS) {
            event_description = "File was accessed";
        }

        if (mask & IN_ATTRIB) {
            event_description = "File metadata changed";
        }

        if (mask & IN_CLOSE_WRITE) {
            event_description = "File opened for writing was closed";
        }

        if (mask & IN_CLOSE_NOWRITE) {
            event_description = "File or directory not opened for writing was closed";
        }

        if (mask & IN_CREATE) {
            event_description = "File/directory created in watched directory";
        }

        if (mask & IN_DELETE) {
            event_description = "File/directory deleted from watched directory";
        }

        if (mask & IN_DELETE_SELF) {
            event_description = "File/directory deleted";
        }

        if (mask & IN_MODIFY) {
            event_description = "File was modified";
        }

        if (mask & IN_MOVE_SELF) {
            event_description = "File/directory was moved";
        }

        if (mask & IN_MOVED_FROM) {
            event_description = "File moved out of directory";
        }

        if (mask & IN_MOVED_TO) {
            event_description = "File moved into directory";
        }

        if (mask & IN_OPEN) {
            event_description = "File or directory was opened";
        }
        
        char log_message[256];
        snprintf(log_message, 256, "%s: %s (Severity %d)\n", item, event_description, severity);

        write(log_fd, log_message, strlen(log_message));
    }
}