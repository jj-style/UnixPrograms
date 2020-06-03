/* Copies a file using system calls */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>

#define BSIZE 16384

int main(int argc, char *argv[]) {
    int fin, fout;
    char buf[BSIZE]; // buffer for copying data between files
    char files[2][PATH_MAX]; // array containing the source and destination absolute file names
    int count,i;

    if (argc != 3) {
        fprintf(stderr, "usage: cp_sys source destination\n");
        exit(1);
    }

    for(i=0;i<2;++i)
        realpath(argv[i+1],files[i]); /* build absolute path if not given for the arguments */

    if ( (fin = open(files[0], O_RDONLY)) < 0 ) {
        perror(files[0]);
        exit(1);
    }
    if ( (fout = open(files[1], O_WRONLY | O_CREAT, 0644)) < 0 ) {
        perror(files[1]);
        exit(1);
    }

    while ( (count = read(fin,buf,BSIZE)) > 0 )
        write(fout,buf,count);

    close(fin);
    close(fout);
    return 0;
}