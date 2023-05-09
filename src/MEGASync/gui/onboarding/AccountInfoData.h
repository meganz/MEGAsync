#ifndef ACCOUNTINFODATA_H
#define ACCOUNTINFODATA_H

#include "QTMegaRequestListener.h"

#include <memory>

class AccountInfoData : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

    Q_PROPERTY(AccountType type MEMBER mType)
    Q_PROPERTY(QString totalStorage MEMBER mTotalStorage)
    Q_PROPERTY(QString usedStorage MEMBER mUsedStorage)

public:
    enum AccountType {
        ACCOUNT_TYPE_FREE = 0,
        ACCOUNT_TYPE_PROI = 1,
        ACCOUNT_TYPE_PROII = 2,
        ACCOUNT_TYPE_PROIII = 3,
        ACCOUNT_TYPE_LITE = 4,
        ACCOUNT_TYPE_BUSINESS = 100,
        ACCOUNT_TYPE_PRO_FLEXI = 101
    };
    Q_ENUM(AccountType)

    explicit AccountInfoData(QObject *parent = 0);

    void onRequestStart(mega::MegaApi*,
                        mega::MegaRequest *request) override;

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest *request,
                         mega::MegaError* error) override;

public slots:
    void requestAccountInfoData();
    void aboutToBeDestroyed();

signals:
    void accountDetailsChanged();

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;

    AccountType mType;
    QString mTotalStorage;
    QString mUsedStorage;

};

#endif // ACCOUNTINFODATA_H
