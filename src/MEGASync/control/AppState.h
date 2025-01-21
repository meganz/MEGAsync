#ifndef APPSTATE_H
#define APPSTATE_H

// This Class is an embryonic proposal of a way to centrally handle the app state.
// For now only "Nominal", "Reloading" and "Fatal error" states are handled, as using this
// class app-wide would require more work, way out of the scope of the ticket
// it was introduced in.
// Those states have been identified to react to MegaEvent::EVENT_FATAL_ERROR and
// MegaEvent::EVENT_RELOAD
//
// To use from another class T:
// Create:
// - a void onAppStateChanged(AppState::AppStates oldAppState, AppState::AppStates newAppState);
// slot,
// - a void requestAppState(AppState::AppStates newAppState); signal,
// and connect like this:
// connect(this, &T::requestAppState,
//         AppState::instance().get(), &AppState::setAppState);
// connect(AppState::instance().get(), &AppState::appStateChanged,
//         this, &T::onAppStateChanged);
// emit the signal when T wants to change the app state, and react to app state changes in the slot.
//
// TODO: Proper state machine, with state transition checks and so on.
// TODO: Future states to consider: APP_FINISHED, LOGING_IN

#include <QObject>

#include <memory>

class AppState: public QObject
{
    Q_OBJECT
    Q_PROPERTY(AppStates appState MEMBER mAppState READ getAppState WRITE setAppState NOTIFY
                   appStateChanged)

public:
    enum AppStates
    {
        NOMINAL,
        RELOADING,
        FATAL_ERROR,
        FINISHED,
    };
    Q_ENUM(AppStates)

    static std::shared_ptr<AppState> instance();

    AppState(const AppState&) = delete;
    AppState& operator=(const AppState&) = delete;

    Q_INVOKABLE AppStates getAppState() const;

public slots:
    Q_INVOKABLE void setAppState(AppStates newState);

signals:
    void appStateChanged(AppStates oldAppState, AppStates newAppState);

private:
    explicit AppState();

    AppStates mAppState;
};

Q_DECLARE_METATYPE(AppState::AppStates)

#endif // APPSTATE_H
