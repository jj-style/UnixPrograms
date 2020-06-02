/* Copies a file using C standard library */

#include <stdio.h>
#include <string.h>
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

    for(i=0;i<2;++i) {
        /* remove trailing / in source and destination if there is one */
        if (argv[i+1][strlen(argv[i+1])-1] == '/')
            argv[i+1][strlen(argv[i+1])-1] = '\0';
        
        /* build absolute path if not given */
        if (argv[i+1][0] == '/') {
            strncpy(files[i],argv[i+1],PATH_MAX);
        } else {
            getcwd(files[i],PATH_MAX);
            strcat(files[i],"/");
            strcat(files[i],argv[i+1]);
        }
    }

    fin = fopen(files[0], "r");
    fout = fopen(files[1], "w");
    
    while ( (count = fread(buf,1,BSIZE,fin)) > 0 ) /* read one lot of BSIZE bytes */
        fwrite(buf,1,count,fout); /* write one lot of BSIZE bytes */

    fclose(fin);
    fclose(fout);

    return 0;
}