#include "ProxyStatsEventHandler.h"

#include "Utilities.h"
#include "MegaApplication.h"

#include <QProcessEnvironment>

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventType type,
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

void ProxyStatsEventHandler::sendTrackedEvent(AppStatsEvents::EventType type, bool fromInfoDialog)
{
    if(!mMegaApi)
    {
        return;
    }

    if(mUpdateViewID)
    {
        if(!fromInfoDialog || (fromInfoDialog && mLastInfoDialogEventSent))
        {
            mViewID = mMegaApi->generateViewId();
            mUpdateViewID = false;
        }
        else if(fromInfoDialog && !mLastInfoDialogEventSent)
        {
            mLastInfoDialogEventSent = true;
        }
    }

    sendEvent(type, AppStatsEvents::getEventMessage(type), true, mViewID);
}

void ProxyStatsEventHandler::sendTrackedEvent(AppStatsEvents::EventType type,
                                              const QObject* senderObj,
                                              const QObject* expectedObj,
                                              bool fromInfoDialog)
{
    if (senderObj != nullptr && senderObj == expectedObj)
    {
        sendTrackedEvent(type, fromInfoDialog);
    }
}

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventType type,
                                       const char* message,
                                       bool addJourneyId,
                                       const char* viewId)
{
    if(!mMegaApi || !canSend())
    {
        return;
    }

    int eventType = AppStatsEvents::getEventType(type);
    if(eventType == -1)
    {
        mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING,
                      "Trying to send an event with not valid type");
    }
    else if(QString::fromUtf8(message).isEmpty())
    {
        mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING,
                      "Trying to send an event with not valid message");
    }
    else
    {
        mMegaApi->sendEvent(eventType, message, addJourneyId, viewId);
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
