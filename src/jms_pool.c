#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    int max_jobs = 0;
    char *path = NULL;
    char *pipe_read_name = NULL;
    char *pipe_write_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            max_jobs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            path = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            pipe_read_name = argv[++i];
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            pipe_write_name = argv[++i];
        }
    }

    // Open named pipes for communication with coordinator
    int fd_in = open(pipe_read_name, O_RDONLY);
    if (fd_in == -1) {
        perror("Pool: Error opening input pipe");
        exit(1);
    }
    
    int fd_out = open(pipe_write_name, O_WRONLY);
    if (fd_out == -1) {
        perror("Pool: Error opening output pipe");
        exit(1);
    }

    int jobs_handled = 0; 
    char buffer[512];

    // Main loop: process jobs until max limit reached
    while (jobs_handled < max_jobs) {

        // Check for finished child processes and notify coordinator
        int status;
        pid_t finished_job_pid;
        while ((finished_job_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            char finish_msg[256];
            sprintf(finish_msg, "FINISHED PID: %d", finished_job_pid);
            write(fd_out, finish_msg, strlen(finish_msg) + 1);
        }
        
        // Read job command from coordinator
        if (read(fd_in, buffer, sizeof(buffer) - 1) > 0) {

            jobs_handled++;

            pid_t job_pid = fork();

            if (job_pid < 0) {
                perror("Pool: Fork failed");
                continue;
            }

            if (job_pid == 0) { // Child process: execute the job
                
                // Create timestamped output directory
                time_t t = time(NULL);
                struct tm *tm_info = localtime(&t);
                char folder_name[1024];

                sprintf(folder_name, "%s/out_%d_%ld_%04d%02d%02d_%02d%02d%02d", 
                        path, jobs_handled, (long)getpid(),
                        tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

                if (mkdir(folder_name, 0777) == -1) {
                    perror("Job: mkdir failed");
                    exit(1);
                }

                // Prepare stdout and stderr files
                char out_file[1100], err_file[1100];
                sprintf(out_file, "%s/stdout", folder_name);
                sprintf(err_file, "%s/stderr", folder_name);

                int out_fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                int err_fd = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (out_fd < 0 || err_fd < 0) {
                    perror("Job: Failed to open output files");
                    exit(1);
                }

                // Redirect stdout and stderr to files
                dup2(out_fd, STDOUT_FILENO);
                dup2(err_fd, STDERR_FILENO);

                close(out_fd);
                close(err_fd);
                close(fd_in);
                close(fd_out);

                // Execute command via shell
                execl("/bin/sh", "sh", "-c", buffer, (char *)NULL);
                
                perror("Job: execl failed");
                exit(1);

            } else { // Parent process: notify coordinator
                
                char response[256];
                sprintf(response, "JobID: %d, PID: %d", jobs_handled, job_pid);
                write(fd_out, response, strlen(response) + 1);
            }
        }
    }

    close(fd_in);
    close(fd_out);
    return 0;
}