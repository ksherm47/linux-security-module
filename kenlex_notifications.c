#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "kenlex_notifications.h"
#include "kenlex_structures.h"

int kenlex_log_init(char* log_filename) {
    log_fd = open(log_filename, O_RDWR | O_CREAT | O_APPEND, 0644);

    if (log_fd < 0) {
        return -1;
    }
    log_file = log_filename;
    return 0;
}

int kenlex_log_error(char* error_msg) {
    int rc = -1;
    if (log_fd > 0) {

        time_t temp = time(NULL);
        struct tm* timeptr = localtime(&temp);
        char date_string[100];

        strftime(date_string, 100, "%D %r", timeptr);

        char error_message[512];
        snprintf(error_message, 512, "(%s) ERROR: %s\n", date_string, error_msg);

        write(log_fd, error_message, strlen(error_message));
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

int kenlex_email_event(char* item, int mask, int severity, char** email_addresses, int num_email_addresses, int event_type) {
    int rc = 0;

    char email_message[512];
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

    char email_address_str[4096];
    int off = 0;
    for (int i = 0; i < num_email_addresses; i++) { 
        memcpy(email_address_str + off, email_addresses[i], strlen(email_addresses[i]));
        off += strlen(email_addresses[i]);

        if(i < num_email_addresses - 1) {
            memcpy(email_address_str + off, ",", 1);
            off++;
        }
    }
    email_address_str[off] = '\0';

    char hostname[254];
    gethostname(hostname, sizeof(hostname));


    snprintf(email_message, sizeof(email_message), "File %s event triggered for item %s on host %s (Severity %d). Log attached.", event_type_str, item, hostname, severity);
    char system_cmd[8192];
    snprintf(system_cmd, sizeof(system_cmd), "echo \"%s\" | mail -s \"Kenlex File Event Alert\" -a \"From:Kenlex Notifications\" -A %s %s\0", email_message, log_file, email_address_str);
    rc = system(system_cmd);

    return rc;
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

int kenlex_email_frequency(char* item, int frequency, long int time_frame, char** email_addresses, int num_email_addresses, int event_type) {
    return -1;
}