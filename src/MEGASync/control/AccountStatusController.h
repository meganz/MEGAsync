#ifndef ACCOUNTSTATUSCONTROLLER_H
#define ACCOUNTSTATUSCONTROLLER_H

#include "QTMegaListener.h"
#include "Preferences/Preferences.h"

#include <QObject>

#include <memory>

class mega::MegaApi;

class BlockedAccount : public QObject
{

public:
    explicit BlockedAccount(QObject* parent = nullptr);
    virtual void setAccountBlocked(int blockState);
    void showVerifyAccountInfo();
    bool isAccountBlocked() const;

protected:
    int mBlockState;
};

class BlockedAccountLinux : public BlockedAccount
{
    Q_OBJECT

public:
    explicit BlockedAccountLinux(QObject* parent = nullptr);
    void setAccountBlocked(int blockState) override;

private slots:
    void onTimeout();

private:
    QTimer* mTimer;
};


class AccountStatusController : public QObject, public mega::MegaListener
{
    Q_OBJECT
public:
    AccountStatusController(QObject* parent = nullptr);

    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

    void whyAmIBlocked();
    bool isAccountBlocked() const;

private:
    BlockedAccount* mBlockedAccount;
    mega::MegaApi* mMegaApi;
    mega::QTMegaListener *mDelegateListener;
    std::shared_ptr<Preferences> mPreferences;
    bool mQueringWhyAmIBlocked;
};


#endif // ACCOUNTSTATUSCONTROLLER_H
