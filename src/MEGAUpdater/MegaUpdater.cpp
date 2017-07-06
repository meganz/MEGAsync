#include <iostream>
#include "UpdateTask.h"
#include "megaapi.h"
#include "Preferences.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    mega::MegaApi *megaApi = new mega::MegaApi(CLIENT_KEY, (const char*)NULL, USER_AGENT);
#ifdef DEBUG
    mega::MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_MAX);
    mega::MegaApi::setLogToConsole(true);
#else
    mega::MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_DEBUG);
#endif

    UpdateTask *updater = new UpdateTask(megaApi);
    updater->checkForUpdates();
    printf ("\n\nProcess finishes at %ju\n\n", (uintmax_t)time(0));

    return 0;
}
