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
public:
    AccountStatusController(QObject* parent = nullptr);

    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

    void whyAmIblocked();

private:
    BlockedAccount* mBlockedAccount;
    mega::MegaApi* mMegaApi;
    mega::QTMegaListener *mDelegateListener;
    std::shared_ptr<Preferences> mPreferences;
    bool mQueringWhyAmIBlocked;
};

class BlockedAccount
{
public:
    explicit BlockedAccount(QObject* parent = nullptr);
    virtual void setAccountBlocked(int blockState);

private:
    int mBlockState;
};

class BlockedAccountLinux : public BlockedAccount
{
public:
    explicit BlockedAccountLinux(QObject* parent = nullptr);
    void setAccountBlocked(int blockState) override;

private slots:
    void timeout();

private:
    QTimer* mTimer;
};

#endif // ACCOUNTSTATUSCONTROLLER_H
