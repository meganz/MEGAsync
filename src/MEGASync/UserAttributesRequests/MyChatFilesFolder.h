#ifndef MYCHATFILESFOLDER_A
#define MYCHATFILESFOLDER_A

#include <control/UserAttributesManager.h>

namespace UserAttributes
{
class MyChatFilesFolder : public AttributeRequest
{
    Q_OBJECT

public:
    MyChatFilesFolder(const QString& userEmail) : AttributeRequest(userEmail),
    mMyChatFilesFolderHandle(mega::INVALID_HANDLE){}

    static std::shared_ptr<const MyChatFilesFolder> requestMyChatFilesFolder();

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    AttributeRequest::RequestInfo fillRequestInfo() override;

    bool isAttributeReady() const override;
    const mega::MegaHandle& getMyChatFilesFolderHandle() const;

signals:
    void attributeReady(const mega::MegaHandle& handle);

private:
    mega::MegaHandle mMyChatFilesFolderHandle;
};
}

#endif // MYCHATFILESFOLDER_A
