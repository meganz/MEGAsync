#ifndef MYBACKUPSHANDLEATTRIBUTESREQUEST_H
#define MYBACKUPSHANDLEATTRIBUTESREQUEST_H

#include <control/UserAttributesManager.h>
#include <mega/bindings/qt/QTMegaRequestListener.h>

Q_DECLARE_METATYPE(mega::MegaHandle)

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
    QString getNodeLocalizedPath(QString path) const;

    void createMyBackupsFolderIfNeeded();

signals:
    void attributeReady(mega::MegaHandle h);

private slots:
    void onMyBackupsFolderReady(mega::MegaHandle h);

private:
    mega::MegaHandle mMyBackupsFolderHandle;
    QString mMyBackupsFolderPath;
    bool mCreationRequested;
    std::unique_ptr<CreateMyBackupsListener> mCreateBackupsListener;
};

class CreateMyBackupsListener : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    CreateMyBackupsListener();
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *error) override;
    virtual ~CreateMyBackupsListener();

signals:
    void backupFolderCreated(mega::MegaHandle h);

private:
    mega::QTMegaRequestListener *mDelegateListener;
};
}

#endif // MYBACKUPSHANDLEATTRIBUTESREQUEST_H
