/* move a file */
#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    char files[2][PATH_MAX];
    int i;
    struct stat sb;

    if (argc != 3) { // two non-option
        fprintf(stderr, "usage: mv source destination\n");
        exit(1);
    }

    for(i=0;i<2;++i)
        realpath(argv[i+1],files[i]); /* build absolute path if not given for the arguments */

    /* do some checks first */
    if ( stat(files[0],&sb) < 0 ) {
        perror(argv[0]);
        exit(1);
    }

    if (S_ISDIR(sb.st_mode)) {
        fprintf(stderr, "mv: cannot move '%s': Is a directory\n",argv[0]);
        exit(1);
    }

    if (!(sb.st_mode & S_IWUSR)) {
        fprintf(stderr, "mv: do not have user write permission to move '%s'\n", argv[0]);
        exit(1);
    }

    /* hard link files */
    if( link(files[0],files[1]) == -1) {
        perror("unable to move");
        exit(1);
    }

    /* remove the original */
    if( unlink(files[0]) == -1) {
        perror("unable to delete original");
        exit(1);
    }

    return 0;
}