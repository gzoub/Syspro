#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    int pool_count = 0; // Counter for the number of job pools

    jms_request msg;


    fd_set readfds;
    int max_fd;

    // Read and process incoming requests in a loop
    while(1){

        FD_ZERO(&readfds);
        FD_SET(fd_in, &readfds); // Add console input to monitored file descriptors
        max_fd = fd_in;

        for (int i = 0; i < pool_count; i++) {
            if (pool_info[i].current_jobs != -1) { // Only monitor active pools
                FD_SET(pool_info[i].fd_read, &readfds);
                if (pool_info[i].fd_read > max_fd) max_fd = pool_info[i].fd_read;
            }
        }

        int wait_status;
        pid_t p_pid;
        while ((p_pid = waitpid(-1, &wait_status, WNOHANG)) > 0) {
            for (int i = 0; i < pool_count; i++) {
                if (pool_info[i].pool_pid == p_pid) {
                    pool_info[i].current_jobs = -1; // Mark pool as terminated
                    close(pool_info[i].fd_read);
                    close(pool_info[i].fd_write);
                }
            }
        }

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            continue;
        }

        if(FD_ISSET(fd_in, &readfds))
        {
            // Read a request from the input pipe
            if(read(fd_in, &msg, sizeof(msg)) > 0){

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

        for (int i = 0; i < pool_count; i++) {
            if (pool_info[i].current_jobs != -1 && FD_ISSET(pool_info[i].fd_read, &readfds)) {
                char pool_msg[256];
                int n = read(pool_info[i].fd_read, pool_msg, sizeof(pool_msg)-1);
                if (n > 0) {
                    pool_msg[n] = '\0';
                    printf("[Coord] Pool %d says: %s\n", pool_info[i].pool_pid, pool_msg);
                    // TODO: Update job status tracking when job completion message received
                }
            }
        }
    }
}

