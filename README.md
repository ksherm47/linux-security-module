# KENLEX Notification Library and Tool

This repository contains the source code for building the Kenlex Notifcation tool suite and Tool used for monitoring file reads, writes, and otherwise general access attempts in the background leveraging the inotify module.

## Dependencies
The user will need the ``mailutils`` and ``ssmtp`` libraries installed on thier system in order to send notification emails. These can be installed using the following command:

``sudo apt install mailutils ssmtp``

Once these libraries are installed, the user will have to edit the ssmtp configuration file:

``sudo vim /etc/ssmtp/ssmtp.conf``

The following lines will need to be appended (or updated) in this configuration file:

```
mailhub=smtp.gmail.com:587
AuthUser=kenlexnotify@gmail.com
AuthPass=kenlexnotify522
FromLineOverride=YES
UseSTARTTLS=YES
```

## Building

In order to build the Kenlex library, simply run:

``make``

in the root kenlex directory. By default, the generated library ``libkenlex.so`` will be built under the ``lib`` directory. This can be overridden by passing in a value for ``LIB_DIR`` when running ``make``.

When linking this library to a program, simply append ``-L <directory of libkenlex.so> -lkenlex -lpthread`` to your ``gcc`` command. Additionally, be sure to add the directory of ``kenlex.h`` with the ``-I`` option if this file does not exist in your current project directory.

## Usage Documentation

Below are the functions used to use the kenlex monitoring tools and interface with the monitoring threads and settings:

```
// Regarding Items to Monitor
int setup_kenlex_monitor(pthread_t* monitor_thread);
int kenlex_cleanup();
int kenlex_add_path(const char* path);
int listen_for_kenlex_events(int kenlex_wd);
int stop_listening(int kenlex_wd);

// Regarding Processing Initialization
int begin_event_processing(pthread_t* processing_thread);

// Regarding Settings Interfacing
void add_item_setting(char* item, int item_len, struct event_settings_struct event_settings, int setting_type);

void set_log_severity(int severity);
int get_log_severity();
void set_email_severity(int severity);
int get_email_severity();
void add_email_address(char* email, int address_len);
```

