/* rm command to delete a single file */
#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>

#define MAX_ARGS 100

int rflag = 0;

void rm_dir(char *path) {
    struct stat sb;
    struct dirent *info;
    DIR *d;

    if ( stat(path,&sb) < 0 ) {
        perror(basename(path));
        return;
    }

    if (S_ISDIR(sb.st_mode)) {
        if(rflag) {
            char subpath[PATH_MAX];
            if ( (d = opendir(path)) == NULL ) {
                perror(path);
                exit(1);
            }
            while( (info=readdir(d)) != NULL ) {
                if (!strcmp(info->d_name,".") || !strcmp(info->d_name,"..")) continue;
                sprintf(subpath,"%s/%s",path,info->d_name);
                rm_dir(subpath);
            }
            rmdir(path);
        } else {
            fprintf(stderr, "rm: cannot remove '%s': Is a directory\n",basename(path));
            return;
        }
    } else {
        if (!(sb.st_mode & S_IWUSR)) {
            fprintf(stderr, "rm: do not have user write permission to remove '%s'\n", basename(path));
            return;
        }
        unlink(path);
    }
}

int main(int argc, char *argv[]) {
    int c,i;
    char fullpaths[MAX_ARGS][PATH_MAX]; /* allow MAX_ARGS files to be passed in as command line arguments */

    opterr = 0; // delete this to see getopt's own error messages

    while( (c=getopt(argc,argv,"arl")) != EOF ) {
        switch(c) {
            case 'r':
                rflag = 1;
                break;
            case '?':
                fprintf(stderr, "invalid option -%c\n",optopt);
        }
    }

    argv += optind; // move past the options
    argc -= optind; // argc is now the number of non-option arguments and argv[0] is the first

    if (argc < 1) { /* one or more non-option */
        fprintf(stderr, "usage: rm [-r] file [file file ...]\n");
        exit(1);
    }

    for(i=0;i<argc&&i<MAX_ARGS;++i)
        realpath(argv[i],fullpaths[i]);

    for(i=0;i<argc&&i<MAX_ARGS;++i)
        rm_dir(fullpaths[i]);

    return 0;
}