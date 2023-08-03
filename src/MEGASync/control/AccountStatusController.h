#ifndef ACCOUNTSTATUSCONTROLLER_H
#define ACCOUNTSTATUSCONTROLLER_H

#include "QTMegaListener.h"
#include "Preferences/Preferences.h"

#include <QObject>

#include <memory>

class mega::MegaApi;

class AccountStatusController : public QObject, public mega::MegaListener
{
    Q_OBJECT
    Q_PROPERTY(int blockedState READ getBlockedState NOTIFY accountBlocked)
public:
    AccountStatusController(QObject* parent = nullptr);

    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

    void whyAmIBlocked();
    int getBlockedState() const;
    bool isAccountBlocked() const;
    void loggedIn();

signals:
    void accountBlocked(/*int blocked*/);

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
