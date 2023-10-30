#ifndef ACCOUNTINFODATA_H
#define ACCOUNTINFODATA_H

#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"

#include <memory>

class AccountInfoData : public QObject, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT

    Q_PROPERTY(AccountType type MEMBER mType NOTIFY accountDetailsChanged)
    Q_PROPERTY(QString totalStorage MEMBER mTotalStorage NOTIFY accountDetailsChanged)
    Q_PROPERTY(QString usedStorage MEMBER mUsedStorage NOTIFY accountDetailsChanged)
    Q_PROPERTY(bool newUser MEMBER mNewUser NOTIFY accountDetailsChanged)

public:
    enum AccountType {
        ACCOUNT_TYPE_NOT_SET = -1,
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

public slots:
    void requestAccountInfoData();

signals:
    void accountDetailsChanged();

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;

    AccountType mType;
    QString mTotalStorage;
    QString mUsedStorage;
    bool mNewUser;

    static const long long INITIAL_SPACE;

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest *request,
                         mega::MegaError* error) override;
    void onAccountUpdate(mega::MegaApi *api) override;
};

#endif // ACCOUNTINFODATA_H
