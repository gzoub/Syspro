#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      
#include <sys/stat.h>


int main(int argc, char *argv[]){

    // Clean up any existing pipes
    unlink(PIPE_IN);
    unlink(PIPE_OUT);

    // Create named pipes
    if(mkfifo(PIPE_IN,0666) == -1){
        perror("Error creating pipe in");
        exit(1);
    }
    if(mkfifo(PIPE_OUT,0666) == -1){
        perror("Error creating pipe out");
        exit(1);
    }

    // Open input pipe in non-blocking
    int fd = open(PIPE_IN , O_RDONLY | O_NONBLOCK);
    if(fd == -1){
        perror("Error opening pipe in");
        exit(1);
    }

    jms_request msg;

    // Process incoming requests
    while(read(fd , &msg ,sizeof(jms_request))){

        switch(msg.type){
            case submit: 
                printf("submit request, Job : %s\n", msg.job_command);
                break;
            case status:
                printf("status request, Job id :  %i\n" , msg.job_id);
                break;
            case status_all:
                printf("status_all request, n : %i\n"  , msg.n_time);   
                break;
            case show_active:
                printf("show_active request");
                break;
            case show_pools:
                printf("show_pools request");
                break;
            case show_finished:
                printf("show_finished request, Job id :  %i\n" , msg.job_id);
                break;
            case suspend:
                printf("suspend request, Job id :  %i\n" , msg.job_id);
                break;
            case resume:
                printf("resume request, Job id :  %i\n" , msg.job_id);
                break;
            case shutdown:
                printf("shutdown request");
                // Cleanup and exit
                close(fd);
                unlink(PIPE_IN);
                unlink(PIPE_OUT);
                break;
            default :
                printf("no valid request");
        }
    }
}
