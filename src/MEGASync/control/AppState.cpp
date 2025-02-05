#include "AppState.h"

#include "DialogOpener.h"
#include "megaapi.h"
#include "QmlManager.h"

#include <QQmlEngine>
#include <QString>
#include <QVariant>

std::shared_ptr<AppState> AppState::instance()
{
    static std::shared_ptr<AppState> appState(new AppState());
    return appState;
}

AppState::AppStates AppState::getAppState() const
{
    return mAppState;
}

void AppState::setAppState(AppStates newState)
{
    auto* sender(QObject::sender());
    QString appStateChangeRequester;
    if (sender != nullptr)
    {
        appStateChangeRequester = QString::fromLatin1(sender->metaObject()->className());
    }
    else
    {
        appStateChangeRequester = QLatin1String("Unknown");
    }

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       QString::fromLatin1("AppState change request to: %1 by: %2")
                           .arg(QVariant::fromValue<AppState::AppStates>(newState).toString(),
                                appStateChangeRequester)
                           .toUtf8()
                           .constData());

    if (mAppState != newState)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                           QString::fromLatin1("Switching AppState from: %1 to: %2")
                               .arg(QVariant::fromValue<AppState::AppStates>(mAppState).toString(),
                                    QVariant::fromValue<AppState::AppStates>(newState).toString())
                               .toUtf8()
                               .constData());
        auto oldState = mAppState;
        mAppState = newState;
        switch (mAppState)
        {
            case AppState::RELOADING:
            // Fallthrough
            case AppState::FATAL_ERROR:
            {
                DialogOpener::closeAllDialogs();
                break;
            }
            case AppState::NOMINAL:
            // Fallthrough
            case AppState::FINISHED:
            {
                break;
            }
        }
        emit appStateChanged(oldState, mAppState);
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                           QString::fromLatin1("AppState already is: %1")
                               .arg(QVariant::fromValue<AppState::AppStates>(mAppState).toString())
                               .toUtf8()
                               .constData());
    }
}

AppState::AppState():
    QObject{nullptr},
    mAppState{AppState::NOMINAL}
{
    qmlRegisterUncreatableType<AppState>(
        "AppState",
        1,
        0,
        "AppState",
        QLatin1String("Warning AppState: not allowed to be instantiated"));
    QmlManager::instance()->setRootContextProperty(QLatin1String("appStateAccess"), this);
}
