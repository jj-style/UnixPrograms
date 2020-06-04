/* Copies a file */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define BSIZE 16384

int main(int argc, char *argv[]) {
    FILE *fin, *fout;
    int fdin, fdout;
    char buf[BSIZE]; // buffer for copying data between files
    char files[2][PATH_MAX]; // array containing the source and destination absolute file names
    int count,i,c;
    char mode[4] = {"std"};

    opterr = 0; // delete this to see getopt's own error messages
    while( (c=getopt(argc,argv,"m:")) != EOF ) {
        switch(c) {
            case 'm':
                if ((strcmp("std",optarg) && strcmp("sys",optarg) && strcmp("map",optarg))) {
                    fprintf(stderr, "unknown mode %s. Using default.\n",optarg);
                    strcpy(mode,"std");
                } else
                    strcpy(mode,optarg);
                break;
            case '?':
                fprintf(stderr, "invalid option -%c\n",optopt);
        }
    }

    argv += optind; // move past the options
    argc -= optind; // argc is now the number of non-option arguments and argv[0] is the first

    if (argc != 2) {
        fprintf(stderr, "usage: cp [-m mode] source destination\n");
        exit(1);
    }

    for(i=0;i<2;++i)
        realpath(argv[i],files[i]); /* build absolute path if not given for the arguments */

    if(!strcmp("std",mode)) {
        printf("copying mode std\n");
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
    } else if(!strcmp("sys",mode)) {
        printf("copying mode sys\n");
        if ( (fdin = open(files[0], O_RDONLY)) < 0 ) {
            perror(files[0]);
            exit(1);
        }
        if ( (fdout = open(files[1], O_WRONLY | O_CREAT, 0644)) < 0 ) {
            perror(files[1]);
            exit(1);
        }

        while ( (count = read(fdin,buf,BSIZE)) > 0 )
            write(fdout,buf,count);

        close(fdin);
        close(fdout);
    } else if(!strcmp("map",mode)) {
        printf("copying mode map\n");
        char *src, *dst;
        size_t size;
        if ( (fdin = open(files[0], O_RDONLY)) < 0 ) {
            perror(files[0]);
            exit(1);
        }

        size = lseek(fdin,0,SEEK_END); /* ofset of 0 from the end of the file to return its size*/

        src = mmap(NULL,size,PROT_READ,MAP_PRIVATE,fdin,0);
        if (src == MAP_FAILED) {
            perror("mmap");
            exit(2);
        }

        
        if ( (fdout = open(files[1], O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0 ) {
            perror(files[1]);
            exit(1);
        }

        if (ftruncate(fdout,size) == -1) { /* if can't make output file same size as input */
            perror("ftruncate");
            exit(3);
        }

        dst = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fdout,0);
        if (dst == MAP_FAILED) {
            perror("mmap");
            exit(4);
        }

        memcpy(dst,src,size); /* copy bytes between mappings */
        if (msync(dst,size,MS_SYNC) == -1) { /* flush changes of mapping */
            perror("msync");
            exit(5);
        }

        close(fdin);
        close(fdout);
    }

    return 0;
}