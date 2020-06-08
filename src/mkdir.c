/* create an empty directory */
#include <stdio.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_ARGS 100

int main(int argc, char *argv[]) {
    char fullpaths[MAX_ARGS][PATH_MAX]; /* allow MAX_ARGS files to be passed in as command line arguments */
    int i;

    if(argc < 2) { /* one or more non-option arguments */
        fprintf(stderr,"usage: mkdir dir [dir dir ...]\n");
        exit(1);
    }

    for(i=0;i<argc-1&&i<MAX_ARGS;++i)
        realpath(argv[i+1],fullpaths[i]);

    for(i=0;i<argc-1&&i<MAX_ARGS;++i)
        if(mkdir(fullpaths[i],0775) < 0)
            perror(basename(fullpaths[i]));
}