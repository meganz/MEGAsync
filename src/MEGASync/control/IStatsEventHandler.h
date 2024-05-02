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
        , mInfoDialogVisible(false)
        , mCurrentView(nullptr)
        , updateViewID(true)
    {}

    virtual ~IStatsEventHandler() = default;

    Q_INVOKABLE virtual void sendEvent(AppStatsEvents::EventTypes type,
                                       const QStringList& args = QStringList(),
                                       bool encode = false) = 0;
    Q_INVOKABLE virtual void sendTrackedEvent(int type) = 0;

protected:
    mega::MegaApi* mMegaApi;
    const char* mViewID;
    bool mInfoDialogVisible;
    QObject* mCurrentView;
    bool updateViewID;

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
                mCurrentView = nullptr;
                updateViewID = true;
            }
        }
        else
        {
            if(!mInfoDialogVisible && MegaSyncApp->isInfoDialogVisible())
            {
                mInfoDialogVisible = true;
                updateViewID = true;
            }
            else if(event->type() == QEvent::WindowActivate || event->type() == QEvent::FocusIn)
            {
                if(mCurrentView != obj)
                {
                    mCurrentView = obj;
                    updateViewID = true;
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }
};

#endif // ISTATSEVENTHANDLER_H
