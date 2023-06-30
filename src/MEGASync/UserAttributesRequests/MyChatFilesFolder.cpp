#include "MyChatFilesFolder.h"

#include "megaapi.h"
#include "mega/types.h"
#include "MegaApplication.h"

namespace UserAttributes
{
//MY CHAT FILES REQUEST
//
//
void MyChatFilesFolder::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        mMyChatFilesFolderHandle = incoming_request->getNodeHandle();

        if(isAttributeReady())
        {
            emit attributeReady(getMyChatFilesFolderHandle());
        }
    }
}

void MyChatFilesFolder::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_MY_CHAT_FILES_FOLDER);
}

AttributeRequest::RequestInfo MyChatFilesFolder::fillRequestInfo()
{
    std::function<void()> cameraRequestFunc = [](){
        MegaSyncApp->getMegaApi()->getMyChatFilesFolder();
};
    QSharedPointer<ParamInfo> avatarParamInfo(new ParamInfo(cameraRequestFunc, QList<int>()<<mega::MegaError::API_OK));

    ParamInfoMap paramInfo({{mega::MegaApi::USER_ATTR_MY_CHAT_FILES_FOLDER, avatarParamInfo}});
    RequestInfo ret(paramInfo, QMap<int64_t, int>({{mega::MegaUser::CHANGE_TYPE_MY_CHAT_FILES_FOLDER, mega::MegaApi::USER_ATTR_MY_CHAT_FILES_FOLDER}}));
    return ret;
}

bool MyChatFilesFolder::isAttributeReady() const
{
    return mMyChatFilesFolderHandle != mega::INVALID_HANDLE;
}

const mega::MegaHandle &MyChatFilesFolder::getMyChatFilesFolderHandle() const
{
    return mMyChatFilesFolderHandle;
}

std::shared_ptr<const MyChatFilesFolder> MyChatFilesFolder::requestMyChatFilesFolder()
{
    return UserAttributesManager::instance().requestAttribute<MyChatFilesFolder>();
}

}//end namespace UserAttributes
