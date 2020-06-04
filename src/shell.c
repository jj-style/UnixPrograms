/* A basic shell program supporting piping */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>

int parse_input_and_args(char **upstream, char **downstream, char *fwrite, int *append) {
    char buf[ARG_MAX];

    downstream[0] = NULL;
    strcpy(fwrite,"");
    *append = 0;
    printf("> ");
    if(fgets(buf,ARG_MAX,stdin) == NULL)
        return -1;
    buf[strlen(buf)-1] = '\0'; /* strip newline from input */

    *upstream++ = strtok(buf," "); /* definitely one thing entered as didn't return above from NULL input */
    while( *upstream = strtok(NULL," ") ) {
        if (!strcmp(*upstream, "|")) {
            *upstream = NULL;  /* "|" consumed, set upstream to NULL and begin parsing for the downstream */
            while (*downstream++ = strtok(NULL, " ")) { /* parses up to the end of the string setting the end of downstream to NULL */
                ;
            }
            return 1;
        } else if (!strcmp(*upstream, ">") || !strcmp(*upstream, ">>")) {
            *append = !strcmp(*upstream,">>");
            *upstream = NULL;
            char *tok;
            strcpy(fwrite,strtok(NULL, ""));
            return 1;
        }
        ++upstream;
    }
    return 1;
}

void redirect_to_file(void (*function)(), char *fout, int append) {
    int p[2];
    pipe(p);

    if (fork() == 0) {
        /* upstream child */
        dup2(p[1],1);
        close(p[0]);

        (*function)();
        printf("after function\n");
        exit(0); // in case function passed doesn't exit the child
    } 
    if(fork() == 0) {
        /* downstream child */
        dup2(p[0],0);
        close(p[1]);

        int fd, count;
        char buf[16384];

        if( (fd = open(fout, O_WRONLY|O_CREAT| (append?O_APPEND:O_TRUNC), 0644)) == -1) {
            perror(fout);
            exit(1);
        }

        while ( (count = read(0,buf,16384)) > 0 ){
            write(fd,buf,count);
        }
        close(fd);
        exit(0);
    } 
    else {
        close(p[0]);
        close(p[1]);
        wait(0);
        wait(0);
    }
}

int main() {

    char *upstream[ARG_MAX/2];
    char *downstream[ARG_MAX/2];
    char fwrite[PATH_MAX];
    int append;
    int p[2];

    void execute_upstream() {
        execvp(upstream[0], upstream);
        fprintf(stderr, "command '%s' not found\n",upstream[0]);
        exit(1);
    }

    void execute_downstream() {
        execvp(downstream[0], downstream);
        fprintf(stderr, "command '%s' not found\n",downstream[0]);
        exit(1);
    }

    while(parse_input_and_args(upstream, downstream, fwrite, &append) > 0) {        
        if (downstream[0] == NULL) {
            /* no downstream = no pipe so just normal command */
            if(fork() == 0) {
                /* child */
                if (strlen(fwrite) == 0)
                    execute_upstream();
                else {
                    redirect_to_file(execute_upstream,fwrite,append);
                }
            } else
                wait(0); /* don't do anything with return status of child yet */
        } else {
            /* is downstream = pipe so fork again */
            pipe(p);
            /* fork twice, creating two children one for each command so not running exec in main */
            if(fork() == 0) {
                /* parent - upstream */
                dup2(p[1],1);
                close(p[0]);

                execute_upstream();
            } 
            if(fork() == 0) {
                /* child - downstream */
                dup2(p[0],0);
                close(p[1]);

                execute_downstream();
            }
            else {
                /* close ends of pipes */
                close(p[0]);
                close(p[1]);
                /* wait for both children to terminate */
                wait(0);
                wait(0);
            }
        }
    }

    return 0;
}