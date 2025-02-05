#include "ProxyStatsEventHandler.h"

#include <QProcessEnvironment>

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventType type,
                                       const QStringList& args,
                                       bool encode)
{
    QString message = AppStatsEvents::getEventMessage(type, args);
    send(type, encode ? encodeMessage(message) : message);
}

void ProxyStatsEventHandler::sendTrackedEvent(AppStatsEvents::EventType type,
                                              bool fromInfoDialog)
{
    if(!mMegaApi)
    {
        return;
    }

    updateTrackInfo(fromInfoDialog);
    const bool addJourneyId = true;
    send(type, AppStatsEvents::getEventMessage(type), addJourneyId, mViewID);
}

void ProxyStatsEventHandler::sendTrackedEventArg(AppStatsEvents::EventType type,
                                                 const QStringList& args,
                                                 bool fromInfoDialog)
{
    if(!mMegaApi)
    {
        return;
    }

    updateTrackInfo(fromInfoDialog);
    const bool addJourneyId = true;
    send(type, AppStatsEvents::getEventMessage(type, args), addJourneyId, mViewID);
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

void ProxyStatsEventHandler::send(AppStatsEvents::EventType type,
                                  const QString& message,
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
    else if(message.isEmpty())
    {
        mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING,
                      "Trying to send an event with not valid message");
    }
    else
    {
        mMegaApi->sendEvent(eventType, message.toUtf8().constData(), addJourneyId, viewId);
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

QString ProxyStatsEventHandler::encodeMessage(const QString& msg) const
{
    QByteArray base64stats = msg.toUtf8().toBase64();
    base64stats.replace('+', '-');
    base64stats.replace('/', '_');
    while (base64stats.size() && base64stats[base64stats.size() - 1] == '=')
    {
        base64stats.resize(base64stats.size() - 1);
    }
    return QString::fromUtf8(base64stats);
}

void ProxyStatsEventHandler::updateTrackInfo(bool fromInfoDialog)
{
    if (!mUpdateViewID)
    {
        return;
    }

    if (!fromInfoDialog || mLastInfoDialogEventSent)
    {
        mViewID = mMegaApi->generateViewId();
        mUpdateViewID = false;
    }
    else
    {
        mLastInfoDialogEventSent = true;
    }
}

