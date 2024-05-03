#ifndef ISTATSEVENTHANDLER_H
#define ISTATSEVENTHANDLER_H

#include "AppStatsEvents.h"
#include "MegaApplication.h"

#include "megaapi.h"

#include <QEvent>

class IStatsEventHandler : public QObject
{
    Q_OBJECT

public:
    IStatsEventHandler(mega::MegaApi* megaApi, QObject* parent = nullptr)
        : QObject(parent)
        , mMegaApi(megaApi)
        , mViewID(nullptr)
        , mCurrentView(nullptr)
        , mInfoDialogVisible(false)
        , mUpdateViewID(true)
        , mLastInfoDialogEventSent(true)
    {
        if(megaApi)
        {
            mViewID = mMegaApi->generateViewId();
        }
    }

    virtual ~IStatsEventHandler() = default;

    Q_INVOKABLE virtual void sendEvent(AppStatsEvents::EventTypes type,
                                       const QStringList& args = QStringList(),
                                       bool encode = false) = 0;

    Q_INVOKABLE virtual void sendTrackedEvent(int type,
                                              bool fromInfoDialog = false) = 0;

    virtual void sendTrackedEvent(int type,
                                  const QObject* senderObj,
                                  const QObject* expectedObj,
                                  bool fromInfoDialog = false) = 0;

protected:
    mega::MegaApi* mMegaApi;
    const char* mViewID;
    QObject* mCurrentView;
    bool mInfoDialogVisible;
    bool mUpdateViewID;
    bool mLastInfoDialogEventSent;

    virtual void sendEvent(AppStatsEvents::EventTypes type,
                           const char* message,
                           bool addJourneyId = false,
                           const char* viewId = nullptr) = 0;


    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if(mInfoDialogVisible)
        {
            if(!MegaSyncApp->isInfoDialogVisible())
            {
                mInfoDialogVisible = false;
                mLastInfoDialogEventSent = false;
                mUpdateViewID = true;
                QString msg(QString::fromUtf8("Fertest : dialog hide"));
                mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING, msg.toUtf8().constData());
            }
        }
        else
        {
            if(!mInfoDialogVisible && MegaSyncApp->isInfoDialogVisible())
            {
                mInfoDialogVisible = true;
                mUpdateViewID = true;
                mLastInfoDialogEventSent = true;
                QString msg(QString::fromUtf8("Fertest : dialog show"));
                mMegaApi->log(mega::MegaApi::LOG_LEVEL_WARNING, msg.toUtf8().constData());
            }
            else if(event->type() == QEvent::WindowActivate || event->type() == QEvent::FocusIn)
            {
                if(mCurrentView != obj)
                {
                    mCurrentView = obj;
                    mUpdateViewID = true;
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }
};

#endif // ISTATSEVENTHANDLER_H
