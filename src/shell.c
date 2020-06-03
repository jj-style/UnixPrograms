/* A basic shell program supporting piping */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT_LENGTH 100

int parse_input_and_args(char **upstream, char **downstream) {
    char buf[MAX_INPUT_LENGTH];

    downstream[0] = NULL;
    printf("> ");
    if(fgets(buf,MAX_INPUT_LENGTH,stdin) == NULL)
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
        }
        ++upstream;
    }
    return 1;
}

int main() {

    char *upstream[MAX_INPUT_LENGTH];
    char *downstream[MAX_INPUT_LENGTH];
    int p[2];

    while(parse_input_and_args(upstream, downstream) > 0) {        
        
        if (downstream[0] == NULL) {
            /* no downstream = no pipe so just normal command */
            if(fork() == 0) {
                /* child */
                execvp(upstream[0], upstream);
                fprintf(stderr, "command '%s' not found\n",upstream[0]);
                exit(1);
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

                execvp(upstream[0], upstream);
                fprintf(stderr, "command '%s' not found\n",upstream[0]);
                exit(1);
            } 
            if(fork() == 0) {
                /* child - downstream */
                dup2(p[0],0);
                close(p[1]);
                execvp(downstream[0], downstream);
                fprintf(stderr, "command '%s' not found\n",downstream[0]);
                exit(1);
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