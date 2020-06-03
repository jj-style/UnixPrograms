/* Copies a file using C standard library */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

#define BSIZE 16384

int main(int argc, char *argv[]) {
    FILE *fin, *fout;
    char buf[BSIZE]; // buffer for copying data between files
    char files[2][PATH_MAX]; // array containing the source and destination absolute file names
    int count,i;

    if (argc != 3) {
        fprintf(stderr, "usage: cp_std source destination\n");
        exit(1);
    }

    for(i=0;i<2;++i)
        realpath(argv[i+1],files[i]); /* build absolute path if not given for the arguments */

    if ( (fin = fopen(files[0], "r")) == NULL ) {
        perror(files[0]);
        exit(2);
    }
    if ( (fout = fopen(files[1], "w")) == NULL ) {
        perror(files[1]);
        exit(3);
    }
    
    while ( (count = fread(buf,1,BSIZE,fin)) > 0 ) /* read one lot of BSIZE bytes */
        fwrite(buf,1,count,fout); /* write one lot of BSIZE bytes */

    fclose(fin);
    fclose(fout);

    return 0;
}