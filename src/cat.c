/* Print file to the standard output */

#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>

#define BSIZE 16384

int main(int argc, char *argv[]) {
    FILE *f;
    char buf[BSIZE]; // buffer for copying data between files
    char file[PATH_MAX]; // absolute file name
    int count;

    if (argc != 2) {
        fprintf(stderr, "usage: cat file\n");
        exit(1);
    }

    realpath(argv[1], file);
    
    if ( (f=fopen(file, "r")) == NULL ) {
        perror(file);
        exit(2);
    }
    
    while ( (count = fread(buf,1,BSIZE,f)) > 0 ) /* read one lot of BSIZE bytes */
        fwrite(buf,1,count,stdout); /* write out one lot of BSIZE bytes */

    fclose(f);
    return 0;
}