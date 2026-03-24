#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      
#include <sys/stat.h>



int main(int argc, char *argv[]){


    char *ops_file_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && (i + 1 < argc)) {
            ops_file_path = argv[i + 1]; 
    }

    int fd_in= open(PIPE_IN, O_WRONLY);
    if(fd_in == -1){
        perror("Error opening pipe in");
        exit(1);
    }

    int fd_out = open(PIPE_OUT, O_RDONLY | O_NONBLOCK);
    if(fd_out == -1){
        perror("Error opening pipe out");
        exit(1);
    }

    FILE *input_src = stdin;
    if(ops_file_path != NULL){
        input_src = fopen(ops_file_path,"r");

    }
    char line[500];
    while(fgets(line,sizeof(line),input_src)){
        
        line[strcspn(line, "\n")] = 0;
        jms_request req;

        if(strncmp(line,"submit ",7) == 0){
            req.type = submit;
            strncpy(req.job_command , line + 7,MAX_CMD_LEN);
        }

        if(strncmp(line,"status ",7) == 0){
            req.type = status;
            req.job_id = atoi(line + 7);
        }

        if(strncmp(line,"status-all ",11) == 0){
            req.type = status_all;
            req.n_time = atoi(line + 11);
        }

        if(strncmp(line,"show-active",11) == 0){
            req.type = show_active;
        }

        if(strncmp(line,"show-pools",10) == 0){
            req.type = show_pools;
        }

        if(strncmp(line,"show-finished ",14) == 0){
            req.type = show_finished;
            req.job_id = atoi(line + 14);
        }

        if(strncmp(line,"suspend ",8) == 0){
            req.type = suspend;
            req.job_id = atoi(line + 8);
        }

        if(strncmp(line,"resume ",7) == 0){
            req.type = resume;
            req.job_id = atoi(line + 7);
        }

        if(strncmp(line,"shutdown",8) == 0){
            req.type = shutdown;
        }

        write(fd_in , &req , sizeof(jms_request));

    }

}