#include "ProxyStatsEventHandler.h"

#include "Utilities.h"
#include "MegaApplication.h"

#include <QProcessEnvironment>

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventTypes type,
                                       const QStringList& args,
                                       bool encode)
{
    QString message(QString::fromUtf8(AppStatsEvents::getEventMessage(type)));
    for (const QString& arg : args)
    {
        message = message.arg(arg);
    }

    sendEvent(type, encode ? encodeMessage(message).constData() : message.toUtf8().constData());
}

void ProxyStatsEventHandler::sendTrackedEvent(int type)
{
    if(updateViewID)
    {
        mViewID = mMegaApi->generateViewId();
        updateViewID = false;
    }

    sendEvent(type, AppStatsEvents::getEventMessage(type), true, mViewID);
}

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventTypes type,
                                       const char* message,
                                       bool addJourneyId,
                                       const char* viewId)
{
    if(canSend() && mMegaApi)
    {
        if(QString::fromUtf8(message).isEmpty())
        {
            mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING,
                          "Trying to send an event with not valid message");
        }
        else
        {
            mMegaApi->sendEvent(type, message, addJourneyId, viewId);
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

QByteArray ProxyStatsEventHandler::encodeMessage(const QString& msg) const
{
    QByteArray base64stats = msg.toUtf8().toBase64();
    base64stats.replace('+', '-');
    base64stats.replace('/', '_');
    while (base64stats.size() && base64stats[base64stats.size() - 1] == '=')
    {
        base64stats.resize(base64stats.size() - 1);
    }
    return base64stats;
}
