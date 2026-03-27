#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include "requests.h"


// Job Management System Coordinator
// Receives and processes job requests from clients via named pipes
int main(int argc, char *argv[]){

    int max_jobs_per_pool = atoi(argv[2]); // Maximum number of jobs per pool

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

    // Open input pipe for reading client requests
    int fd_in = open(PIPE_IN , O_RDONLY);
    if(fd_in == -1){
        perror("Error opening pipe in");
        exit(1);
    }

    // Open output pipe for writing responses back to clients
    int fd_out = open(PIPE_OUT , O_WRONLY);
    if(fd_out == -1){
        perror("Error opening pipe out");
        exit(1);
    }
    PoolInfo pool_info[100]; // Array to store information about job pools
    int *pool_count = 0; // Counter for the number of job pools

    jms_request msg;

    // Read and process incoming requests in a loop
    while(read(fd_in , &msg , sizeof(jms_request))){

        switch(msg.type){
            case submit:
                // Handle job submission request
                submit_job(&msg, pool_info, &pool_count, max_jobs_per_pool, argv[1], fd_out); 
                               
                break;
            case status:
                printf("status request, Job id :  %i\n" , msg.job_id);

                break;
            case status_all:
                printf("status_all request, n : %i\n"  , msg.n_time);   

                break;
            case show_active:
                printf("show_active request\n");

                break;
            case show_pools:
                printf("show_pools request\n");

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
                printf("shutdown request\n");
                // Clean up resources and exit
                close(fd_in);
                close(fd_out);
                unlink(PIPE_IN);
                unlink(PIPE_OUT);
                return 0;
            default:
                // Handle unrecognized request types
                printf("no valid request\n");
        }
    }
}

