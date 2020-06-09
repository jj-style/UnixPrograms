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

/* linked list structure for holding background jobs */
struct bg_job {
    char *pname;
    pid_t pid;
    struct bg_job *next;
};

struct bg_job *head = NULL;

void add_bg_job(char* name, pid_t pid) {
    struct bg_job *new_bg_job = (struct bg_job*) malloc(sizeof(struct bg_job));
    new_bg_job -> next = head;
    new_bg_job -> pid = pid;
    // new_bg_job -> pname = name;
    new_bg_job->pname = (char *) malloc(strlen(name));
    strcpy(new_bg_job->pname,name);
    head = new_bg_job;
}

void rmv_bg_job(pid_t pid) {
    struct bg_job *it, *to_rmv, *prv;
    if(head != NULL && head->pid == pid) {
        to_rmv = head;
        head = head->next;
    } else {
        for(it=head;it!=NULL;prv=it,it=it->next) {
            if(it->pid == pid) {
                to_rmv = it;
                break;
            }
        }
        prv->next = to_rmv->next;
    }
    free(to_rmv->pname);
    free(to_rmv);
}

void list_bg_jobs(struct bg_job **it) {
    if(*it==NULL)
        return;
    list_bg_jobs(&((*it)->next));
    printf("%s [%d]\n",(*it)->pname,(*it)->pid);
}

int is_bg_job(pid_t pid) {
    struct bg_job *it;
    if(head != NULL && head->pid == pid) {
        return 1;
    } else {
        for(it=head;it->next!=NULL;it=it->next) {
            if(it->next->pid == pid)
                return 1;
        }
    }
    return 0;
}

/* end of linked list structure related things */

int _cd(char **args);
int _help(char **args);
int _kill(char **args);
int _jobs(char **args);

static char *built_in_commands[] = {"cd","kill","jobs","help"};
static int(*built_in_functions[])(char **) = {_cd,_kill,_jobs,_help};
static int num_built_ins = sizeof(built_in_commands) / sizeof(char *);

int _cd(char **args) {
    if (chdir(args[1]) < 0) {
        perror(args[1]);
    }
    return 1;
}

int _kill(char **args) {
    pid_t p = atoi(args[1]);
    if( kill(p, SIGKILL) == -1)
        perror(args[1]);
    else {
        rmv_bg_job(p);
        printf("KILLED PID: %d\n", p);
    }
}

int _jobs(char **args) {
    list_bg_jobs(&head);
    return 1;
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

int parse_input_and_args(char **upstream, char **downstream, char *fwrite, int *append, int *bg) {
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
    *bg = 0;

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
                    return 1;
                }
                ++downstream;
            }
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
    *bg = !strcmp(upstream[i-1],"&");
    if(*bg)
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
    int append, bg;
    pid_t finish_pid;
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
    while(parse_input_and_args(upstream, downstream, fwrite, &append, &bg) > 0) {
        finish_pid = waitpid(-1, NULL, WNOHANG);
        if (finish_pid > 0) {
            printf("FINISHED PID %d\n", finish_pid);
            rmv_bg_job(finish_pid);
        }
        if (!upstream[0]) continue;
        if (downstream[0] == NULL) {
            /* no downstream = no pipe so just normal command 
                before fork, check if built in command to execute as must execute in main
            */

            if (built_in(upstream) != -1)
                continue;

            pid_t pid;
            if((pid = fork()) == 0) {
                /* child */
                if (strlen(fwrite) == 0)
                    execute_upstream();
                else
                    redirect_to_file(execute_upstream,fwrite,append);
            } else {
                if(!bg)
                    waitpid(pid,NULL,0); /* don't do anything with return status of child yet */
                else {
                    printf("background process started: %d\n", pid);
                    add_bg_job(upstream[0], pid);
                }
            }
        } else {
            /* is downstream = pipe so fork again */
            pid_t ppid,cpid;
            pipe(p);
            /* fork twice, creating two children one for each command so not running exec in main */
            if((ppid = fork()) == 0) {
                /* parent - upstream */
                dup2(p[1],1);
                close(p[0]);

                execute_upstream();
            } 
            if((cpid = fork()) == 0) {
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
                waitpid(ppid,NULL,0);
                waitpid(cpid,NULL,0);
            }
        }
    }

    return 0;
}