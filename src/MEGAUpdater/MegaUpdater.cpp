#include <iostream>
#include "UpdateTask.h"
#include "megaapi.h"
#include "Preferences.h"

#include <iostream>
#include <sstream>
#include <time.h>

using namespace std;
using namespace mega;

int main(int argc, char *argv[])
{
#ifdef DEBUG
    MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_INFO);
    MegaApi::setLogToConsole(true);
#else
    MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_WARNING);
#endif

    ostringstream oss;
    oss  << "Process started at " << time(0);
    string msg = oss.str();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, msg.c_str());

    MegaApi *megaApi = new MegaApi(CLIENT_KEY, (const char*)NULL, USER_AGENT);

    UpdateTask *updater = new UpdateTask(megaApi);
    updater->checkForUpdates();

    oss.clear();
    oss.seekp(0);
    oss  << "Process finished at " << time(0) << ends;
    msg = oss.str();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, msg.c_str());

    delete updater;
    delete megaApi;
    return 0;
}
