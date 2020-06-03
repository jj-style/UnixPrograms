/* Copies a file using mmap */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h> /* needed for memcpy */
#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>

int main(int argc, char *argv[]) {

    char *src, *dst;
    int fin, fout;
    char files[2][PATH_MAX]; // array containing the source and destination absolute file names
    size_t size; // size of the file to copy
    int i;

    if (argc != 3) {
        fprintf(stderr, "usage: cp_mmap source destination\n");
        exit(1);
    }

    for(i=0;i<2;++i)
        realpath(argv[i+1],files[i]); /* build absolute path if not given for the arguments */

    if ( (fin = open(files[0], O_RDONLY)) < 0 ) {
        perror(files[0]);
        exit(1);
    }

    size = lseek(fin,0,SEEK_END); /* ofset of 0 from the end of the file to return its size*/

    src = mmap(NULL,size,PROT_READ,MAP_PRIVATE,fin,0);
    if (src == MAP_FAILED) {
        perror("mmap");
        exit(2);
    }

    
    if ( (fout = open(files[1], O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0 ) {
        perror(files[1]);
        exit(1);
    }

    if (ftruncate(fout,size) == -1) { /* if can't make output file same size as input */
        perror("ftruncate");
        exit(3);
    }

    dst = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fout,0);
    if (dst == MAP_FAILED) {
        perror("mmap");
        exit(4);
    }

    memcpy(dst,src,size); /* copy bytes between mappings */
    if (msync(dst,size,MS_SYNC) == -1) { /* flush changes of mapping */
        perror("msync");
        exit(5);
    }

    return 0;

}