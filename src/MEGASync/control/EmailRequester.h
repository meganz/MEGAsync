#ifndef EMAILREQUESTER_H
#define EMAILREQUESTER_H

#include <mega/bindings/qt/QTMegaGlobalListener.h>
#include <mega/bindings/qt/QTMegaRequestListener.h>

#include <QMap>
#include <QMutex>
#include <QObject>

class EmailRequester : public QObject, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    static EmailRequester* instance();
    ~EmailRequester() override;

    QString getEmail(mega::MegaHandle userHandle, bool forceRequest = false);
    void addEmailTracking(mega::MegaHandle userHandle, const QString& email = QString());
    void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users) override;

signals:
    void emailChanged();

private:
    explicit EmailRequester();
    void requestEmail(mega::MegaHandle userHandle);

    struct RequestInfo
    {
        bool requestFinished;
        QString email;
    };

    mega::MegaApi* mMegaApi;
    QMutex mRequestsDataLock;
    QMap<mega::MegaHandle, EmailRequester::RequestInfo> mRequestsData;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    static EmailRequester* mInstance;
};

#endif
