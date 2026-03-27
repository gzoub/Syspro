#ifndef REQUESTS_H
#define REQUESTS_H

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>

void submit_job(jms_request *req, PoolInfo *pool_info, int *pool_count, int max_jobs_per_pool, char *path, int fd_out_console);
void create_pool(PoolInfo *pool_info, int id, int max_jobs, char *path);

#endif

