#ifndef EMAILREQUESTER_H
#define EMAILREQUESTER_H

#include <QObject>

#include <mega/bindings/qt/QTMegaRequestListener.h>

class EmailRequester : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit EmailRequester(mega::MegaHandle userHandle);
    ~EmailRequester() override{};
    void requestEmail();
    void onRequestFinish(mega::MegaApi*, mega::MegaRequest* request, mega::MegaError* error) override;

signals:
    void emailReceived(QString email);

private:
    mega::MegaApi * mMegaApi;
    mega::MegaHandle mUserHandle;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
};

#endif
