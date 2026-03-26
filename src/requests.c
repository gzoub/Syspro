#include "requests.h"


void create_pool(PoolInfo pool_info){
    pool_info.pool_pid = fork();

    if(pool_info.pool_pid == 0){

        execl("./jms_pool", "jms_pool", NULL);
        perror("Error executing pool");
        exit(1);
    } else if(pool_info.pool_pid > 0){

        pool_info.current_jobs = 0;

    } else {
        perror("Error forking process");
        exit(1);
    }



}




void submit_job(jms_request *req , PoolInfo *pool_info, int pool_count, int max_jobs_per_pool){


    printf("Submitting job: %s\n", req->job_command);

    bool pool_found = 0; 
    if(pool_count == 0){

        create_pool(pool_info[pool_count]);
    }else{
        for(int i = 0; i < pool_count; i++){
            if(pool_info[i].current_jobs < max_jobs_per_pool){ 

                printf("Assigning job to pool with PID: %d\n", pool_info[i].pool_pid);

                pool_found = 1;
            }

            
        }
        if(!pool_found){

            create_pool(pool_info[pool_count]);
        }
    }

}