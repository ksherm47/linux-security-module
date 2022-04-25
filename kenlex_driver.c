#include "kenlex.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

const long ONE_DAY = 86400000;
const long ONE_HOUR = 3600000;
const long ONE_MINUTE = 60000;
const long ONE_SECOND = 1000;

long parseTimeframe(char* buffer){
    int lastIndex = strlen(buffer) - 1;
    if(buffer[lastIndex] == 'D'){
        buffer[lastIndex] = '\0';
        return ONE_DAY * atoi(buffer);
    } else if (buffer[lastIndex] == 'H'){
        buffer[lastIndex] = '\0';
        return ONE_HOUR * atoi(buffer);
    } else if (buffer[lastIndex] == 'M'){
        buffer[lastIndex] = '\0';
        return ONE_MINUTE * atoi(buffer);
    } else if (buffer[lastIndex] == 'S'){
        buffer[lastIndex] = '\0';
        return ONE_SECOND * atoi(buffer);
    } else {
        return -1;
    }
}

int addSetting(char* filePath, char* maskType, int severity, int frequency, long timeFrame){
    int fileLen = strlen(filePath);
    int i = 0;
    int mask = 0;
    for(i = 0; i < strlen(maskType); i++) {
        if(maskType[i] == 'W'){
            mask = mask | WRITE;
        } else if (maskType[i] == 'R'){
            mask = mask | READ;
        } else if (maskType[i] == 'A'){
             mask = mask | ACCESS;
        }
    }

    struct event_settings_struct es = {
        .severity = severity,
        .frequency = frequency,
        .time_frame = timeFrame
    };

    add_item_setting(filePath, fileLen, es, mask);

}

int main(int argc, char* argv[]){
    FILE* settingsInput;
    int logSeverity = 0;
    int emailSeverity = 0;
    char timeBuff[8];
    char emailBuff[144];
    int defaultFreq = 0;
    long defaultTimeFrame = 0;
    pthread_t monitor_thread, processing_thread;
    
    // we require one file to be passed in containing overall default settings
    // What number defines log severity, what number defines email severity, a default frequency, a default timeframe
    // and then email addresses to be notified
    // Second file passed in is where they want the log to be
    // potentially a third file containing files to watch and settings to watch for
    if(argc < 3 || argc > 4){
        printf("Usage 1: ./kenlex_driver settings_file log_file\nUsage 2: ./kenlex_driver settings_file log_file watch_file");
        exit(-1);
    }

    settingsInput = fopen(argv[1], "r");
    if(!settingsInput){
        printf("Failure to open settings file. Exiting");
        exit(-1);
    }


    if(fscanf(settingsInput, "%d %d %d %s %s", &logSeverity, &emailSeverity, &defaultFreq, timeBuff, emailBuff) != 5){
        printf("Failure to read from settings file. Check format and give all necessary values");
        exit(-1);
    }

    if(kenlex_log_init(argv[2]) < 0) {
        printf("Failure opening log file. Exiting");
        exit(-1);
    }

    // adjust the global settings
    set_log_severity(logSeverity);
    set_email_severity(emailSeverity);
    add_email_address(emailBuff, strlen(emailBuff));

    // add extra email addresses
    while(fscanf(settingsInput, "%s", emailBuff) == 1){
        add_email_address(emailBuff, strlen(emailBuff));
    }

    printf("Global Settings Created Successfully\n");
    //parse default timeframe value
    defaultTimeFrame = parseTimeframe(timeBuff);
    if(defaultTimeFrame < 0){
        printf("Failure to parse default time frame. Check format");
        exit(-1);
    }

    // set up pthread to pass to setup_kenlex_monitor, if this doesn't work, return error/exit
    if(setup_kenlex_monitor(&monitor_thread) < 0){
        printf("Exiting Kenlex");
        exit(-1);
    }
    begin_event_processing(&processing_thread);

    bool success = true;


    /*/etc/password/file.txt WA 2 1 1S
    * R 1 5 1W
    *
    */






    //if there is a third file passed in, initialize kenelex watches
    if(argc == 4){
        FILE* watchInput = fopen(argv[3], "r");
        if(!watchInput){
            success = false;
        }
        
        size_t lineBuffSize = 256;
        char *lineBuff = (char*)malloc(lineBuffSize * sizeof(char));
        char delim[] = " ";
        char *firstString;
        char *filePath;
        char *nextString;
        int kwd = -1;
        int currSeverity = 0;
        long currTimeFrame = 0;
        int currFreqency = 0;
        bool isContinuation = true;
        //if file opened successfully, read in and process security requests so long as there are still lines in the file, and no issues occure during processing
        while(success && getline(&lineBuff, &lineBuffSize, watchInput) > 0){
            // each line read in must start with a string (either file path, or a setting signal)
            firstString = strtok(lineBuff, delim);
            isContinuation = true;

            //it's possible that instead of a file path, the first string will be a some combination of the letters RWA instead,
            //meaning that they refer to the previous line's file path
            int i = 0;
            if(firstString != NULL && strlen(firstString) < 4){
                for(i = 0; i < strlen(firstString); i++) {
                    if(firstString[i] != 'W' && firstString[i] != 'R' && firstString[i] != 'A'){
                        isContinuation = false;
                    }
                }
            } else {
                isContinuation = false;
            }

            if(isContinuation){
                //if it is a continuation, then the filepath should be the same as it was last time
                //the maskType is currently in firstString
                //need to check for remaining variables (severity, frequency, timeframe) if they are not there, then use defaults
                currSeverity = logSeverity;
                currFreqency = defaultFreq;
                currTimeFrame = defaultTimeFrame;
                nextString = strtok(NULL, delim);
                //if severity is specified
                if(nextString){
                    currSeverity = atoi(nextString);
                    //get frequency
                    nextString = strtok(NULL, delim);
                    //if frequency is specified
                    if(nextString){
                        currFreqency = atoi(nextString);
                        //get time frame
                        nextString = strtok(NULL, delim);
                        //if timeframe is specified
                        if(nextString){
                            currTimeFrame = parseTimeframe(nextString);

                            //if there is more to read in the line, or parsing the time frame failed, than input was formatted badly
                            nextString = strtok(NULL, delim);
                            if(nextString == NULL || currTimeFrame < 0){
                                success = false;
                            }
                        }
                    }
                }
                addSetting(filePath, firstString, currSeverity, currFreqency, currTimeFrame);
            } else {
                //if it's not a continuation, then we can start watching the file from last time now that all its settings have been added
                // we will not reach this point in the code if the last line had it's own file, check if isContinuation is false after while loop
                listen_for_kenlex_events(kwd);
                // file path only changes when we are not in a continuation
                filePath = firstString;
                if(filePath != NULL){
                    kwd = kenlex_add_path(filePath);
                    if(kwd < 0){
                        success = false;
                    }
                } else {
                    success = false;
                }
                // At this point, success is true if there was not an issue reading in the file path
                // If we are successful, then a watch has been added in Kenlex and we need to add settings for it
                if(!(kwd < 0)){
                    //It's possible to have different settings for each action: Read, Write, and Access
                    //The next string should be combinations of WRA, otherwise the user is expecting default settings for all 3, to be applied at log security level
                    char* type = strtok(NULL, delim);
                    //if there was nothing left in the line
                    if(!type){
                        addSetting(filePath, "RWA", logSeverity, defaultFreq, defaultTimeFrame);
                    } else {
                        //the maskType is currently in type
                        //need to check for remaining variables (severity, frequency, timeframe) if they are not there, then use defaults
                        currSeverity = logSeverity;
                        currFreqency = defaultFreq;
                        currTimeFrame = defaultTimeFrame;
                        nextString = strtok(NULL, delim);
                        //if severity is specified
                        if(nextString){
                            currSeverity = atoi(nextString);
                            //get frequency
                            nextString = strtok(NULL, delim);
                            //if frequency is specified
                            if(nextString){
                                currFreqency = atoi(nextString);
                                //get time frame
                                nextString = strtok(NULL, delim);
                                //if timeframe is specified
                                if(nextString){
                                    currTimeFrame = parseTimeframe(nextString);

                                    //if there is more to read in the line, or parsing the time frame failed, than input was formatted badly
                                    nextString = strtok(NULL, delim);
                                    if(nextString == NULL || currTimeFrame < 0){
                                        success = false;
                                    }       
                                }
                            }
                        }
                        addSetting(filePath, type, currSeverity, currFreqency, currTimeFrame);
                    }
                } else {
                    //unsuccessful at getting kwd
                    success = false;
                }
            } 
        }
        //if last line read in was not a continuation, need to start the watch
        if(!isContinuation && success){
            listen_for_kenlex_events(kwd);
        }
        if(!success){
            printf("Failure while processing watch file; continue by using Kenlex interface\n");
        }
    }

    // present the UI menu
    // The Kenlex UI allows the opportunity to add files manually with the same settings options
    // Step 1: input a file path
    // Step 2: get settings until done
    // Step 3: repeat
    char fileBuffUI[256];
    char maskBuffUI[8];
    char timeFrameBuffUI[8];
    char answer = '?';
    int severityUI = 0;
    int frequencyUI = 0;
    int timeFrameUI = 0;
    int kwdUI = 0;

    bool invalidMaskString = false;
    bool doneWithSettings = false;

    
    printf("Welcome to Kenlex File Security Monitoring System\n");
    while(true){
        kwdUI = 0;
        do{
            if(kwdUI < 0){
                printf("Inputted file was invalid. Retry.\n");
            }
            printf("Input the name of the file you want to watch\n");
            scanf("%s", fileBuffUI);
            kwdUI = kenlex_add_path(fileBuffUI);
        } while (kwdUI < 0);
        doneWithSettings = false;
        while(!doneWithSettings) {
            invalidMaskString = false;
            do {
                printf("Which actions for %s would you like to watch?\n", fileBuffUI);
                printf("Write R for Read, W for Write, and/or A for Accces in any order with no spaces: ");
                scanf("%s", maskBuffUI);
                if(strlen(maskBuffUI) < 4){
                    int i = 0;
                    for(i = 0; i < strlen(maskBuffUI); i++) {
                        if(maskBuffUI[i] != 'W' && maskBuffUI[i] != 'R' && maskBuffUI[i] != 'A'){
                            invalidMaskString = true;
                            printf("Inputted string was invalid. Try again.");
                        }
                    }
                } else {
                    invalidMaskString = true;
                    printf("Inputted string was invalid. Try again.");
                }
            } while (invalidMaskString);
        
            printf("Global severity levels\nLogging level = %d\nEmail level = %d\n", logSeverity, emailSeverity);
            printf("What security level should your %s settings be?", maskBuffUI);
            scanf("%d", &severityUI);
            printf("At what frequency do you want to be notified of %s events?\n", maskBuffUI);
            printf("We will ask for a number of occurences and a time frame to calculate overall frequency.");
            printf("Enter number of occurences: ");
            scanf("%d", &frequencyUI);
            timeFrameUI = -1;
            while(timeFrameUI < 0){
                printf("Enter a time frame with a number followed by a letter in terms of [D]ays, [H]ours, [M]inutes, or [S]econds.\n");
                printf("For example, enter 12H for 12 Hours\n");
                scanf("%s", timeFrameBuffUI);
                timeFrameUI = parseTimeframe(timeFrameBuffUI);
            }

            addSetting(fileBuffUI, maskBuffUI, severityUI, frequencyUI, timeFrameUI);

            answer = '?';
            while(answer != 'y' && answer != 'Y' && answer != 'N' && answer != 'n'){
                printf("Are you finished inputting watch settings? [Y/N]: \n");
                scanf("%c", &answer);
            }
            if(answer == 'y' || answer == 'Y'){
                doneWithSettings = true;
                listen_for_kenlex_events(kwdUI);
                printf("%s is being watched in Kenlex\n", fileBuffUI);
            }
        }
    }
    return 0;
}