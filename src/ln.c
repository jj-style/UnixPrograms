/* ln with soft flag options */
#include <getopt.h>
#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int sflag = 0;

int main(int argc, char *argv[]) {
    char files[2][PATH_MAX];
    int c,i,res;
    opterr = 0; // delete this to see getopt's own error messages

    while( (c=getopt(argc,argv,"s")) != EOF ) {
        switch(c) {
            case 's':
                sflag = 1;
                break;
            case '?':
                fprintf(stderr, "invalid option -%c\n",optopt);
        }
    }

    argv += optind; // move past the options
    argc -= optind; // argc is now the number of non-option arguments and argv[0] is the first

    if (argc != 2) { // two non-option
        fprintf(stderr, "usage: ln [-s] target directory\n");
        exit(1);
    }

    for(i=0;i<2;++i) {
        /* remove trailing / in source and destination if there is one */
        if (argv[i][strlen(argv[i])-1] == '/')
            argv[i][strlen(argv[i])-1] = '\0';
        
        /* build absolute path if not given */
        if (argv[i][0] == '/') {
            strncpy(files[i],argv[i],PATH_MAX);
        } else {
            getcwd(files[i],PATH_MAX);
            strcat(files[i],"/");
            strcat(files[i],argv[i]);
        }
    }

    sflag ? (res = link(files[0],files[1])) : (res = symlink(files[0],files[1]));
    if ( res == -1 ) {
        perror("link error");
    }

    return 0;
}