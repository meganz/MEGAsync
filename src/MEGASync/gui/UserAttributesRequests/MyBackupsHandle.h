#ifndef MYBACKUPSHANDLEATTRIBUTESREQUEST_H
#define MYBACKUPSHANDLEATTRIBUTESREQUEST_H

#include <control/UserAttributesManager.h>
#include <mega/bindings/qt/QTMegaRequestListener.h>

namespace UserAttributes
{
class CreateMyBackupsListener;

class MyBackupsHandle : public AttributeRequest
{
    Q_OBJECT

public:

    MyBackupsHandle(const QString& userEmail);

    static std::shared_ptr<MyBackupsHandle> requestMyBackupsHandle();

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *error) override;
    void requestAttribute() override;
    RequestInfo fillRequestInfo() override;

    bool isAttributeReady() const override;

    mega::MegaHandle getMyBackupsHandle() const;
    static QString getMyBackupsLocalizedPath();

    void createMyBackupsFolderIfNeeded();

    static const char* DEFAULT_BACKUPS_ROOT_DIRNAME;

signals:
    void attributeReady(mega::MegaHandle h);

private:
    mega::MegaHandle mMyBackupsFolderHandle;
    bool mCreationRequested;
    std::unique_ptr<CreateMyBackupsListener> mCreateBackupsListener;
};

class CreateMyBackupsListener : public mega::MegaRequestListener
{
public:
    CreateMyBackupsListener();
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *error) override;
    virtual ~CreateMyBackupsListener();

private:

    mega::QTMegaRequestListener *mDelegateListener;
};
}

#endif // MYBACKUPSHANDLEATTRIBUTESREQUEST_H
