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

void ProxyStatsEventHandler::sendTrackedEvent(int type, bool fromInfoDialog)
{
    if(!megaApi)
    {
        return;
    }

    if(mUpdateViewID)
    {
        if(!fromInfoDialog || (fromInfoDialog && mLastInfoDialogEventSent))
        {
            mViewID = mMegaApi->generateViewId();
            mUpdateViewID = false;
            mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING, "Fertest : updateViewID required");
        }
        else if(fromInfoDialog && !mLastInfoDialogEventSent)
        {
            mLastInfoDialogEventSent = true;
            QString msg(QString::fromUtf8("Fertest : sendTrackedEvent : mLastInfoDialogEventSent = true"));
            mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING, msg.toUtf8().constData());
        }
    }

    QString msg(QString::fromUtf8("Fertest : updateViewID : "));
    msg = msg + QString::fromUtf8(mViewID);
    msg = msg + QString::fromUtf8(" - event type:  ");
    msg = msg + QString::number(type);
    mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING, msg.toUtf8().constData());
    //sendEvent(type, AppStatsEvents::getEventMessage(type), true, mViewID);
}

void ProxyStatsEventHandler::sendTrackedEvent(int type,
                                              const QObject* senderObj,
                                              const QObject* expectedObj,
                                              bool fromInfoDialog)
{
    if (senderObj != nullptr && senderObj == expectedObj)
    {
        sendTrackedEvent(type, fromInfoDialog);
    }
}

void ProxyStatsEventHandler::sendEvent(AppStatsEvents::EventTypes type,
                                       const char* message,
                                       bool addJourneyId,
                                       const char* viewId)
{
    if(!megaApi || canSend())
    {
        return;
    }

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
