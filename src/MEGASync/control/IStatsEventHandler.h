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
    IStatsEventHandler(mega::MegaApi* megaApi, QObject* parent = nullptr);

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

    bool eventFilter(QObject* obj, QEvent* event) override;

};

#endif // ISTATSEVENTHANDLER_H
