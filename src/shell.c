/* A basic shell program supporting piping */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <libgen.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/types.h>

int _cd(char **args);
int _help(char **args);
int _kill(char **args);
int _jobs(char **args);

static char *built_in_commands[] = {"cd","kill","jobs","help"};
static int(*built_in_functions[])(char **) = {_cd,_kill,_jobs,_help};
static int num_built_ins = sizeof(built_in_commands) / sizeof(char *);

char bg_jobs[128][128];
int num_bg_jobs;

int _cd(char **args) {
    if (chdir(args[1]) < 0) {
        perror(args[1]);
    }
    return 1;
}

int _kill(char **args) {
    pid_t p = atoi(args[1]);
    printf("KILL PID: %d\n", p);
    if( kill(p, SIGKILL) == -1)
        perror(args[1]);
}

int _jobs(char **args) {
    int i;
    for(i=0;i<num_bg_jobs;++i) {
        printf("%d: %s\n",i+1,bg_jobs[i]);
    }
}

int _help(char **args) {
    int i;
    for(i=0;i<num_built_ins;++i) {
        printf("%s\n",built_in_commands[i]);
    }
}

int built_in(char **command) {
    int i;
    for(i=0;i<num_built_ins;++i) {
        if (!strcmp(built_in_commands[i],command[0]))
            return (*built_in_functions[i])(command);
    }
    return -1;
}

int parse_input_and_args(char **upstream, char **downstream, char *fwrite, int *append, int *bg_up, int *bg_down) {
    static char *buf, *rest;
    static char path[PATH_MAX]; getcwd(path,PATH_MAX);
    static char hname[32]; gethostname(hname,32);
    struct passwd *u; u = getpwuid(getuid());
    char prompt[64];
    int i;

    sprintf(prompt,"%s@%s:%s>: ",u->pw_name,hname,basename(path));

    upstream[0] = NULL;
    downstream[0] = NULL;
    strcpy(fwrite,"");
    *append = 0;
    *bg_up = 0;
    *bg_down = 0;

    if (buf) free(buf);
    buf = readline(prompt);
    if (!buf) {
        printf("\n");
        return -1;
    }
    if (strlen(buf) > 0)
        add_history(buf);
    else
        return 1;
    rest = buf;


    *upstream++ = strtok_r(rest," ",&rest); /* definitely one thing entered as didn't return above from NULL input */
    while( *upstream = strtok_r(rest," ",&rest) ) {
        if (!strcmp(*upstream, "|")) {
            *upstream = NULL;  /* "|" consumed, set upstream to NULL and begin parsing for the downstream */
            while (*downstream = strtok_r(rest, " ",&rest)) { /* parses up to the end of the string setting the end of downstream to NULL */
                if (!strcmp(*downstream, ">") || !strcmp(*downstream, ">>")) {
                    *append = !strcmp(*downstream,">>");
                    *downstream = NULL;
                    strcpy(fwrite,strtok_r(rest, "",&rest));
                    for(i=0;i<sizeof(upstream)/sizeof(char *);++i){
                        if(!downstream[i]) break;
                    }
                    *bg_down = !strcmp(downstream[i-1],"&");
                    if(*bg_down)
                        downstream[i-1] = NULL;
                    return 1;
                }
                ++downstream;
            }
            *bg_down = !strcmp(*downstream,"&");
            return 1;
        } else if (!strcmp(*upstream, ">") || !strcmp(*upstream, ">>")) {
            *append = !strcmp(*upstream,">>");
            *upstream = NULL;
            strcpy(fwrite,strtok_r(rest, "",&rest));
            return 1;
        }
        ++upstream;
    }
    for(i=0;i<sizeof(upstream)/sizeof(char *);++i){
        if(!upstream[i]) break;
    }
    *bg_up = !strcmp(upstream[i-1],"&");
    if(*bg_up)
        upstream[i-1] = NULL;
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
    rl_bind_key('\t', rl_complete);
    char *upstream[ARG_MAX/2];
    char *downstream[ARG_MAX/2];
    char fwrite[PATH_MAX];
    int append, bg_up, bg_down;
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

    while(parse_input_and_args(upstream, downstream, fwrite, &append, &bg_up, &bg_down) > 0) {   
        if (!upstream[0]) continue;
        if (downstream[0] == NULL) {
            /* no downstream = no pipe so just normal command 
                before fork, check if built in command to execute as must execute in main
            */

            if (built_in(upstream) != -1)
                continue;

            char bg_name[128];
            pid_t pid = fork();
            if(pid == 0) {
                /* child */
                if (strlen(fwrite) == 0)
                    execute_upstream();
                else
                    redirect_to_file(execute_upstream,fwrite,append);
            } else {
                if(!bg_up)
                    waitpid(pid,NULL,0); /* don't do anything with return status of child yet */
                else {
                    printf("background process started: %d\n", pid);
                    sprintf(bg_name, "%s[%d]",upstream[0],pid);
                    strcpy(bg_jobs[num_bg_jobs++],bg_name);
                }
            }
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

                if(strlen(fwrite) == 0)
                    execute_downstream();
                else
                    redirect_to_file(execute_downstream,fwrite,append);
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