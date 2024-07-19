#ifndef MYBACKUPSHANDLEATTRIBUTESREQUEST_H
#define MYBACKUPSHANDLEATTRIBUTESREQUEST_H

#include "UserAttributesManager.h"

Q_DECLARE_METATYPE(mega::MegaHandle)

namespace UserAttributes
{
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

    void onRequestFinish(mega::MegaRequest *request, mega::MegaError *error);

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
};
}

#endif // MYBACKUPSHANDLEATTRIBUTESREQUEST_H
