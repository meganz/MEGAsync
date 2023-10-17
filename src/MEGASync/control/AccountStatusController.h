#ifndef ACCOUNTSTATUSCONTROLLER_H
#define ACCOUNTSTATUSCONTROLLER_H

#include "QTMegaListener.h"
#include "Preferences/Preferences.h"

#include <QObject>
#include <megaapi.h>

#include <memory>

class AccountStatusController : public QObject, public mega::MegaListener
{
    Q_OBJECT
    Q_PROPERTY(int blockedState MEMBER mBlockedState READ getBlockedState NOTIFY blockedStateChanged FINAL)

public:
    AccountStatusController(QObject* parent = nullptr);
    virtual ~AccountStatusController() = default;

    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

    void loggedIn();
    void reset();
    Q_INVOKABLE int getBlockedState() const;
    Q_INVOKABLE bool isAccountBlocked() const;
    Q_INVOKABLE void whyAmIBlocked(bool force = false);

signals:
    void blockedStateChanged(int blockState);

private:
    void showVerifyAccountInfo();
    mega::MegaApi* mMegaApi;
    mega::QTMegaListener *mDelegateListener;
    std::shared_ptr<Preferences> mPreferences;
    bool mQueringWhyAmIBlocked;
    int mBlockedState;
    bool mBlockedStateSet;
};


#endif // ACCOUNTSTATUSCONTROLLER_H
