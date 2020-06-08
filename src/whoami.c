/* print username associated with the effective user ID of the process */
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    struct passwd *u;
    int euid;

    if(argc != 1) {
        fprintf(stderr,"usage: whoami\n");
        exit(1);
    }

    euid = geteuid(); /* get effective user ID */
    if( (u=getpwuid(euid)) == NULL) {
        perror("unable to retrieve user from effective ID");
        exit(1);
    }
    printf("%s\n",u->pw_name);
    return 0;
}