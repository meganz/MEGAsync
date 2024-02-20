#ifndef EMAILREQUESTER_H
#define EMAILREQUESTER_H

#include <mega/bindings/qt/QTMegaGlobalListener.h>
#include <mega/bindings/qt/QTMegaRequestListener.h>

#include <QMap>
#include <QObject>
#include <QRecursiveMutex>

class RequestInfo: public QObject
{
    Q_OBJECT

public :
    bool requestFinished;
    QString email;

signals:
    void emailChanged(QString email);
};


class EmailRequester : public QObject, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    static EmailRequester* instance();

    void reset();
    QString getEmail(mega::MegaHandle userHandle);
    RequestInfo* addUser(mega::MegaHandle userHandle, const QString& email = QString());
    void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users) override;

private:
    explicit EmailRequester();
    void requestEmail(mega::MegaHandle userHandle);

    mega::MegaApi* mMegaApi;
    QRecursiveMutex mRequestsDataLock;
    QMap<mega::MegaHandle, std::shared_ptr<RequestInfo>> mRequestsData;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    static EmailRequester* mInstance;
};

#endif
