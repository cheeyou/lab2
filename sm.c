/**
 * CS2106 AY 20/21 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "sm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

sm_status_t *serviceArray;
sm_status_t *arrayPtr;

// Use this function to any initialisation if you need to.
void sm_init(void) {
    serviceArray = malloc(SM_MAX_SERVICES * sizeof(sm_status_t));
    // Thus arrayPtr points to the serviceArray
    arrayPtr = serviceArray;
}

// Use this function to do any cleanup of resources.
void sm_free(void) {
    free(serviceArray);
    free(arrayPtr);
}

// Exercise 1a/2: start services
void sm_start(const char *processes[]) {

    int index = 0, totalPipes = 0, pipeIndex = 0, nextProcessOffset;
    // Initially, assume single-process service
    bool hasPrev = false, hasNext = false;
    pid_t result;

    // Gives us the total number of pipes needed
    for (int i = 0; 1; i++) {
        if (processes[i] == NULL) {
            totalPipes++;
         
            if (processes[i + 1] == NULL) {
                break;
            }
        }
    }

    // correct till here

    int fds[totalPipes * 2];
        
    // set up pipes
    for (int i = 0; i < totalPipes; i++) {
        pipe(&fds[i * 2]);        
    }

    int counter = 0;
    // loop through the chain of commands
    while (true) {
        //printf("first %s %d\n", processes[index], counter);
        if (processes[index] == NULL) {
            break;
        }
        //printf("second %s %d\n", processes[index], counter);
        
        nextProcessOffset = 0;

        while (true) {
            // Signals end of a process that is part of the multi-process service
            //printf("%s\n", processes[index + nextProcessOffset]);
            if (processes[index + nextProcessOffset] == NULL) {
                break;
            }
            nextProcessOffset++;
        }
        //printf("%d %s\n", nextProcessOffset, processes[index + nextProcessOffset + 1]);

        nextProcessOffset++;

        if (processes[index + nextProcessOffset] == NULL) {
            hasNext = false;
        } else if (processes[index + nextProcessOffset] != NULL) {
            hasNext = true;
        }
        //printf("%d %d\n", hasPrev, hasNext);

        result = fork();

        if (result < 0) {
            exit(1);
        }
        else if (result == 0) {
            if (hasPrev) {
                dup2(fds[pipeIndex - 2], 0);
            } 
            if (hasNext) {
                dup2(fds[pipeIndex + 1], 1);
            }
            for(int i = 0; i < (totalPipes * 2); i++){
                close(fds[i]);
            }

            const char* path = processes[index];
            char* const* arguments = (char* const*) (processes + index);
            int test = 0;
            test = execv(path, arguments);
        } 

        else if (result > 0){
            const char *servicePath;
            //+ 1 due to the '\0' at the end
            //servicePath = malloc(strlen(processes[0]) + 1);
            //strcpy(servicePath, processes[0]);
            //Basically make a copy of the file Path to exec
            servicePath = strdup(processes[index]);

            arrayPtr->pid = result;
            arrayPtr->path = servicePath;
            arrayPtr->running = true;
            arrayPtr = arrayPtr + 1;
        }
        // Add them here so they will be accounted for by all processes that are forked.
        hasPrev = true;
        index = index + nextProcessOffset;
        pipeIndex += 2;
        counter++;
    }
    for(int i = 0; i < (totalPipes * 2); i++){
        close(fds[i]);
    }
    return;
}

// Exercise 1b: print service status
size_t sm_status(sm_status_t statuses[]) {
    // Programme updates status when start
    //Need to get all services that were launched
    size_t counter = 0;
    // Remember that an array works like a pointer. Dereferencing a pointer here
    // is like getting the array
    size_t size = sizeof(*serviceArray);

    for (size_t i = 0; i < size; i++) {
        int returnStatus;
        // This is where we do our increment, looping through
        sm_status_t *currentProcess = serviceArray + i;
        int response = waitpid(currentProcess->pid, &returnStatus, WNOHANG);
        // Ignore the initial root process and the case where no unterminated children exists
        if (currentProcess->pid == 0 || response == -1) {
            continue;
        } else { // Here, either unterminated children exist or zombie children
            // We fill this status with our current process
            sm_status_t *statusToFill = statuses + i;
            //If wait() or waitpid() returns because the status of a child process is available, 
            //these functions shall return a value equal to the process ID of the child process for which status is reported
            // Since WNOHANG returns such if a zombie process exists, this will confirm that we have a zombie process
            if (response == currentProcess->pid) {
                currentProcess->running = false;
            }
            statusToFill->pid = currentProcess->pid;
            statusToFill->path = currentProcess->path;
            statusToFill->running = currentProcess->running;
            ++counter;
        }
    }
    // Remember to typecast
    return counter;
}

// Exercise 3: stop service, wait on service, and shutdown
void sm_stop(size_t index) {

}

void sm_wait(size_t index) {
}

void sm_shutdown(void) {
}

// Exercise 4: start with output redirection
void sm_startlog(const char *processes[]) {
}

// Exercise 5: show log file
void sm_showlog(size_t index) {
}
