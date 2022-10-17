#ifndef CAMERAUPLOADFOLDER_H
#define CAMERAUPLOADFOLDER_H

#include <control/UserAttributesManager.h>

namespace UserAttributes
{
class CameraUploadFolder : public AttributeRequest
{
    Q_OBJECT

public:
    CameraUploadFolder(const QString& userEmail) : AttributeRequest(userEmail),
    mCameraUploadFolderHandle(mega::INVALID_HANDLE),
    mCameraUploadFolderSecondaryHandle(mega::INVALID_HANDLE){}

    static std::shared_ptr<const CameraUploadFolder> requestCameraUploadFolder();

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    AttributeRequest::RequestInfo fillRequestInfo() override;

    bool isAttributeReady() const override;
    bool isCameraUploadFolderReady() const;
    bool isCameraUploadFolderSecondaryReady() const;
    const mega::MegaHandle& getCameraUploadFolderHandle() const;
    const mega::MegaHandle& getCameraUploadFolderSecondaryHandle() const;
signals:
    void attributeReady(const mega::MegaHandle& handle);
    void cameraUploadFolderReady(const mega::MegaHandle& handle);
    void cameraUploadFolderSecondaryReady(const mega::MegaHandle& handle);

private:

    mega::MegaHandle mCameraUploadFolderHandle;
    mega::MegaHandle mCameraUploadFolderSecondaryHandle;
};
}

#endif // CAMERAUPLOADFOLDER_H
