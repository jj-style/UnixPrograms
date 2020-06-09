/* compare two files byte by byte */
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <libgen.h>
#include <getopt.h>

int aflag = 0;

int main(int argc, char *argv[] ) {
    FILE *f1, *f2;
    int i, c, line=1, byte=1;
    char files[2][PATH_MAX];
    char in1, in2;

    opterr = 0; // delete this to see getopt's own error messages

    while( (c=getopt(argc,argv,"a")) != EOF ) {
        switch(c) {
            case 'a':
                aflag = 1;
                break;
            case '?':
                fprintf(stderr, "invalid option -%c\n",optopt);
        }
    }

    argv += optind; // move past the options
    argc -= optind; // argc is now the number of non-option arguments and argv[0] is the first

    if (argc != 2) { /* 2 non-option args */
        fprintf(stderr, "usage: cmp [-a] file1 file2\n");
        exit(1);
    }

    for(i=0;i<2;++i)
        realpath(argv[i],files[i]); /* build absolute path if not given for the arguments */

    if( (f1=fopen(files[0],"r")) == NULL) {
        perror(files[0]);
        exit(1);
    }

    if( (f2=fopen(files[1],"r")) == NULL) {
        perror(files[1]);
        exit(1);
    }

    while( (in1=getc(f1)) != EOF && (in2=getc(f2)) != EOF ) {
        if(in1 != in2) {
            printf("%s %s differ: byte %d, line %d\n",basename(files[0]),basename(files[1]) , byte, line);
            if(!aflag)
                exit(1);
        }
        else if(in1 == '\n')
            ++line;
        ++byte;
    }

    fclose(f1);
    fclose(f2);
    
    return 0;
}