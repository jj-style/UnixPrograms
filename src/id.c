#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <string.h>

int main() {
    __uid_t uid; 
    __gid_t gid;
    struct passwd *u;
    struct group *g;
    char *member;

    uid = getuid();     // user ID of running process
    u = getpwuid(uid);  // get the user from user ID
    gid = getgid();     // group ID of running process
    g = getgrgid(gid);  // get the group from the group ID

    printf("uid=%d",uid);
    printf("(%s) " ,u->pw_name);

    printf("gid=%d", gid);
    printf("(%s) ", g->gr_name);

    while( (g=getgrent()) != NULL ) { // loop over all groups
        while( (member=*(g->gr_mem)++) != NULL ) // step over list of group member names of this group
            if (!strcmp(member, u->pw_name))
                printf("%d(%s),",g->gr_gid, g->gr_name);
    }
    printf("\n");
    return 0;
}