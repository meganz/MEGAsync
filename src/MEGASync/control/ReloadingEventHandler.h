#ifndef RELOADINGEVENTHANDLER_H
#define RELOADINGEVENTHANDLER_H

#include "AppState.h"
#include "DesktopNotifications.h"
#include "megaapi.h"

#include <QObject>

#include <memory>

class ReloadingEventHandler: public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<ReloadingEventHandler> instance();

    void processEvent(std::unique_ptr<mega::MegaEvent> event,
                      std::shared_ptr<DesktopNotifications> osNotificatons);

signals:
    void requestAppState(AppState::AppStates newAppState);

private:
    explicit ReloadingEventHandler();
    void clear();

    std::shared_ptr<DesktopNotifications> mOsNotifications;
    bool mReloadingRequested;

private slots:
    void onAppStateChanged(AppState::AppStates oldAppState, AppState::AppStates newAppState);
};

#endif // RELOADINGEVENTHANDLER_H
