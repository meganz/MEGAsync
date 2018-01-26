#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <string.h>
#include <errno.h>


int main(int argc, char *argv[])
{
    char releaseStr[256];
    size_t size = sizeof(releaseStr);
    if (!sysctlbyname("kern.osrelease", releaseStr, &size, NULL, 0)  && size > 0)
    {
        if (strchr(releaseStr,'.'))
        {
            char *origVersion = strdup(releaseStr);
            char *token = strtok(releaseStr, ".");
            if (token && origVersion && strcmp(origVersion, token))
            {
                errno = 0;
                char *endPtr = NULL;
                long majorVersion = strtol(token, &endPtr, 10);
                if (endPtr != token && errno != ERANGE && majorVersion >= INT_MIN && majorVersion <= INT_MAX)
                {
                    if((int)majorVersion < 13) // Older versions from 10.9 (mavericks)
                    {
                        execl("/Applications/MEGAsync.app/Contents/MacOS/MEGADeprecatedVersion", "/Applications/MEGAsync.app/Contents/MacOS/MEGADeprecatedVersion", NULL);
                        free(origVersion);
                        return 0;
                    }
                }
                free(origVersion);
            }
        }
    }

    char buf[16];
    int fd = open("/dev/fsevents", O_RDONLY);
    seteuid(getuid());
    snprintf(buf, sizeof(buf), "%d", fd);
    execl("/Applications/MEGAsync.app/Contents/MacOS/MEGAclient", buf, NULL);
    return 0;
}

