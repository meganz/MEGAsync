#include "ProxyStatsEventHandler.h"

#include "Utilities.h"
#include "MegaApplication.h"

#include <QProcessEnvironment>

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventTypes type)
{
    sendEvent(type, AppStatsEvents::getEventMessage(type));
}

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventTypes type, const char* message)
{
    if(canSend())
    {
        if(QString::fromUtf8(message).isEmpty())
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               "Trying to send a single event with not valid message");
        }
        else
        {
            MegaSyncApp->getMegaApi()->sendEvent(type, message, false, nullptr);
        }
    }
}

bool ProxyStatsEventHandler::canSend() const
{
    /*
    * Usage : declare the list of not allowed conditions to send stats.
    */
#if defined QT_DEBUG
    return false;
#else
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    bool inTestEnvironment = QVariant(env.value(QLatin1String("MEGA_TESTS"), QLatin1String("false"))).toBool();

    if (inTestEnvironment)
    {
        return false;
    }

    return true;
#endif
}
