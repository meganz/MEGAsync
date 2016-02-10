#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    char buf[16];
    int fd = open("/dev/fsevents", O_RDONLY);
    seteuid(getuid());
    snprintf(buf, sizeof(buf), "%d", fd);
    execl("/Applications/MEGAsync.app/Contents/MacOS/MEGAclient", buf, NULL);
    return 0;
}

