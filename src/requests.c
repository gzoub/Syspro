#include "requests.h"

void create_pool(PoolInfo *pool_info, int id, int max_jobs, char* path) {
    // Generate unique pipe names for this pool instance
    sprintf(pool_info->pipe_in_name, "tmp/pool_in_%d", id);
    sprintf(pool_info->pipe_out_name, "tmp/pool_out_%d", id);

    // Clean up any existing pipes with the same names
    unlink(pool_info->pipe_in_name);
    unlink(pool_info->pipe_out_name);

    // Create input pipe (Coordinator -> Pool)
    if(mkfifo(pool_info->pipe_in_name,0666) == -1){
        perror("Error creating pipe in");
        exit(1);
    }
    printf("Created pool with ID %d, pipes: %s (in), %s (out)\n", id, pool_info->pipe_in_name, pool_info->pipe_out_name);
    
    // Create output pipe (Pool -> Coordinator)
    if(mkfifo(pool_info->pipe_out_name,0666) == -1){
        perror("Error creating pipe out");
        exit(1);
    }

    pool_info->pool_pid = fork();

    if (pool_info->pool_pid == 0) {
        // Child process: Execute pool worker
        char n[10];
        sprintf(n, "%d", max_jobs);
        
        // Launch jms_pool with pipe names and configuration
        execl("./jms_pool", "jms_pool", "-n", n, "-l", path, 
              "-r", pool_info->pipe_in_name, "-w", pool_info->pipe_out_name, NULL);
        perror("Exec failed");
        exit(1);
    } 
    else {
        // Parent process: Open pipes and initialize pool metadata
        pool_info->fd_write = open(pool_info->pipe_in_name, O_WRONLY);
        pool_info->fd_read = open(pool_info->pipe_out_name, O_RDONLY);
        pool_info->current_jobs = 0;
    }
}


void submit_job(jms_request *req, PoolInfo *pool_info, int *pool_count, int max_jobs_per_pool, char* path,int fd_out_console) {
    printf("Submitting job: %s\n", req->job_command); 

    int target_pool_index = -1;
    bool pool_found = 0; 

    // Search for available pool with free capacity
    if (*pool_count > 0) {
        for (int i = 0; i < *pool_count; i++) {
            if (pool_info[i].current_jobs < max_jobs_per_pool) { 
                target_pool_index = i;
                pool_found = 1;
                break;
            }
        }
    }

    // If no available pool, create a new one
    if (!pool_found) {
        target_pool_index = *pool_count;
        create_pool(&pool_info[target_pool_index], target_pool_index, max_jobs_per_pool, path); 
        (*pool_count)++;
    }

    // Assign job to selected pool
    printf("Assigning job to pool with PID: %d\n", pool_info[target_pool_index].pool_pid);
    
    // Send command to pool worker
    write(pool_info[target_pool_index].fd_write, req->job_command, strlen(req->job_command) + 1); 
    
    // Update pool's job count
    pool_info[target_pool_index].current_jobs++;

    // Read response from pool
    char response[100];
    read(pool_info[target_pool_index].fd_read,response,sizeof(response));

    // Send result back to console
    write(fd_out_console,response,sizeof(response));

}