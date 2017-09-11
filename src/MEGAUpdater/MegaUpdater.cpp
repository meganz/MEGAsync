#include <iostream>
#include "UpdateTask.h"
#include "megaapi.h"
#include "Preferences.h"

#include <iostream>
#include <sstream>
#include <time.h>

using namespace std;
using namespace mega;

#define MAX_LOG_SIZE 4096
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

#ifdef DEBUG
    MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_INFO);
    MegaApi::setLogToConsole(true);
#else
    MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_WARNING);
#endif

    char log_message[MAX_LOG_SIZE];
    time_t currentTime = time(NULL);
    snprintf(log_message, MAX_LOG_SIZE, "Process started at %s", ctime(&currentTime));
    log_message[strlen(log_message) - 1] = '\0';
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, log_message);
    MegaApi megaApi(CLIENT_KEY, (const char*)NULL, USER_AGENT);
    UpdateTask updater(&megaApi);
    updater.checkForUpdates();
    currentTime = time(NULL);
    snprintf(log_message, MAX_LOG_SIZE, "Process finished at %s", ctime(&currentTime));
    log_message[strlen(log_message) - 1] = '\0';
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, log_message);
    return 0;
}
