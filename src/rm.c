/* rm command to delete a single file */
#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    int c;
    char fullpath[PATH_MAX];
    struct stat sb;

    if (argc != 2) { // one non-option
        fprintf(stderr, "usage: rm file\n");
        exit(1);
    }

    realpath(argv[1],fullpath);

    if ( stat(fullpath,&sb) < 0 ) {
        perror(argv[1]);
        exit(2);
    }

    if (S_ISDIR(sb.st_mode)) {
        fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", argv[1]);
        exit(3);
    }

    if (!(sb.st_mode & S_IWUSR)) {
        fprintf(stderr, "rm: do not have user write permission to remove '%s'\n", argv[1]);
        exit(4);
    }

    unlink(fullpath);

    return 0;
}