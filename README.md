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

## Driver Application

To build the provided Kenlex driver, run:

``make driver``

The resulting kenlex_driver executable can be run with the following arguments:

./kenlex_driver [settings_file] [log_file] (optional)[watch_file]

[settings_file]
This file needs to have whitespace separated inputs for the following global settings:
1) `email severity` - a positive integer where events that happen which are at or above that level lead to an email notification

2) `log severity` - a positive integer where events that happen which are at or above that level are logged in the log file
(We recommend log severity to be lower than email severity)

3) `default frequency amount` - a positive number used in conjuction with `default timeframe` to be a default value for determining frequency events
(frequency events are always logged and emailed and are meant to detect suspicious patterns of behavior. ex: 100 access attempts in 1 second)

4) `default timeframe` - a string representing time using #S, #M, #H, #D to represent an amount of seconds, minutes, hours, or days for the 
default frequency event calculation

5) `email addresses` - up to 10 whitespace separated email addresses to be contacted whenever an email worthy event occurs

Example settings_file
**************
1 3 100 1M name@example.com name2@example.com
**************


[log_file]
This is a file path designating where the kenlex log events will be outputted

[watch_file]
An optional file with file paths and other inputs designating security settings, one per line.
There are multiple ways to format each line.

1) If a line only has a file path, and nothing else, then the file will be added at log severity level for Read, Write, and Access,
    with the default frequency event settings applied for each interaction type.

2) A line can have a file path, followed by other inputs in the following order:
    [interaction_type] - a string comprised of at least one of the characters 'R', 'W', and/or 'A' in any order, representing Read, Write, and Access interaction types
    [severity_level] - a severity level to be applied to the interactions specified by the [access_type]
    [frequency_amount] - similar to `default frequency amount`
    [timeframe] - similar to `default timeframe`

    If this is done, any of the inputs can be left off of the end. Ie: you can provide just [access_type], or just [access_type] and [severity_level], and so on.
    For any values not specified, default values will be applied ("RWA" is the default [access_type] and previously defined `log severity` is default severity level)

3)  A line could have no file path, and instead begin with an [interaction_type] signifying string (and also be followed by the other inputs above, 
    with the same ordering and optional inclusion rules). This is how different settings can be applied to different intereaction types


Example watch_file
***********************
/etc/password W 10 5 1S
R 2
/Desktop/testfile
/Desktop/testfile2 R
***********************

Passing in the above example file leads to Kenlex monitoring /etc/password for any Writes as a severity level of 10, with a frequency event also logged whenever
it's written to 5 times in 1 second.
/etc/password is also monitoring for read events, at severity level 2, with whatever the default frequency event settings are (from the settings_file)
/Desktop/testfile ie monitored for all events at log severity level, with the default frequency settings
/Desktop/testfile2 is monitored for only Read events at log severity level, with the default frequency settings.


