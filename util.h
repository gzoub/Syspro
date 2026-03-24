#ifndef UTIL_H
#define UTIL_H

#define PIPE_IN "jms_in"
#define PIPE_OUT "jms_out"
// Maximum command length
#define MAX_CMD_LEN 256

// Command types for job management system
typedef enum {
    submit     = 1,      // Submit a new job
    status,          // Get status of a specific job
    status_all,      // Get status of all jobs
    show_active,     // Show active jobs
    show_pools,      // Show job pools
    show_finished,   // Show finished jobs
    suspend,         // Suspend a job
    resume,          // Resume a suspended job
    shutdown          // Shutdown the system
} command;

// Request structure for job management system
typedef struct {
    command type;                    // Command type
    int job_id;                      // Job identifier
    int n_time;                      // Time parameter
    char job_command[MAX_CMD_LEN];   // Job command string
} jms_request;

#endif
