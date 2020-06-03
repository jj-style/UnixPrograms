/* rm command to delete a single file */
#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_ARGS 100

void rm_dir(char *fullpath, char *shortpath) {
    struct stat sb;

    if ( stat(fullpath,&sb) < 0 ) {
        perror(shortpath);
        return;
    }

    if (S_ISDIR(sb.st_mode)) {
        fprintf(stderr, "rm: cannot remove '%s': Is a directory\n",shortpath);
        return;
    }

    if (!(sb.st_mode & S_IWUSR)) {
        fprintf(stderr, "rm: do not have user write permission to remove '%s'\n", shortpath);
        return;
    }

    unlink(fullpath);
}

int main(int argc, char *argv[]) {
    int c,i;
    char fullpaths[MAX_ARGS][PATH_MAX]; /* allow MAX_ARGS files to be passed in as command line arguments */

    if (argc < 2) { /* one or more non-option */
        fprintf(stderr, "usage: rm file [file file ...]\n");
        exit(1);
    }

    for(i=0;i<argc-1&&i<MAX_ARGS;++i)
        realpath(argv[i+1],fullpaths[i]);

    for(i=0;i<argc-1&&i<MAX_ARGS;++i)
        rm_dir(fullpaths[i],argv[i+1]);

    return 0;
}