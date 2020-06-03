/* ls with all and recursive flag options */
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/limits.h>

int aflag=0, rflag=0;

void listfile(char *fname) {
    struct stat sb;
    char *filetype[] = {"?","p","c","?","d","?","b","?","-","?","l","?","s"};

    if(stat(fname,&sb) < 0) {
        perror(fname);
        // exit(2);
        return;
    }

    printf("%s", filetype[(sb.st_mode >> 12) & 017]);
    printf("%c%c%c%c%c%c%c%c%c",
    (sb.st_mode & S_IRUSR) ? 'r' : '-',
    (sb.st_mode & S_IWUSR) ? 'w' : '-',
    (sb.st_mode & S_IXUSR) ? 'x' : '-',
    (sb.st_mode & S_IRGRP) ? 'r' : '-',
    (sb.st_mode & S_IWGRP) ? 'w' : '-',
    (sb.st_mode & S_IXGRP) ? 'x' : '-',
    (sb.st_mode & S_IROTH) ? 'r' : '-',
    (sb.st_mode & S_IWOTH) ? 'w' : '-',
    (sb.st_mode & S_IXOTH) ? 'x' : '-'
    );
    printf("%8ld", sb.st_size);
    char *s = ctime(&sb.st_atime);
    s[strlen(s)-1] = '\0';  /* remove trailing newline added by ctime */
    printf("  %s", s);
    printf("  %s\n",fname);
}

void listdir(char *dname) {
    DIR *d;
    struct dirent *info;
    char fullpathname[PATH_MAX];
    struct stat sb;

    if ( (d = opendir(dname)) == NULL ) {
        perror(dname);
        exit(1);
    }
    
    while ((info = readdir(d)) != NULL) {
        if (info->d_name[0] == '.' && !aflag) continue;
            /* build new fullpath */
            strcpy(fullpathname,dname);
            strcat(fullpathname,"/");
            strcat(fullpathname,info->d_name);
            /* recurse if rflag is set but not on "." or ".." */
            stat(fullpathname,&sb);
            if (strcmp(info->d_name,".") && strcmp(info->d_name,"..") && rflag && S_ISDIR(sb.st_mode))
                listdir(fullpathname);
            else
                listfile(fullpathname);
    }
    closedir(d);
}

int main(int argc, char *argv[]) {
    int c;
    char dirname[PATH_MAX];

    opterr = 0; // delete this to see getopt's own error messages

    while( (c=getopt(argc,argv,"ar")) != EOF ) {
        switch(c) {
            case 'a':
                aflag = 1;
                break;
            case 'r':
                rflag = 1;
                break;
            case '?':
                fprintf(stderr, "invalid option -%c\n",optopt);
        }
    }

    argv += optind; // move past the options
    argc -= optind; // argc is now the number of non-option arguments and argv[0] is the first

    if (argc != 1) { // one non-option
        fprintf(stderr, "usage: ls [-a] [-r] directory\n");
        exit(1);
    }

    realpath(argv[0],dirname);
    listdir(dirname);

    return 0;
}