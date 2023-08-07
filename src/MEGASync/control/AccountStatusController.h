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
public:
    AccountStatusController(QObject* parent = nullptr);
    virtual ~AccountStatusController();


    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

    int getBlockedState() const;
    void loggedIn();
    void reset();
    Q_INVOKABLE bool isAccountBlocked() const;
    Q_INVOKABLE void whyAmIBlocked(bool force = false);

signals:
    void accountBlocked(int blocked);

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
