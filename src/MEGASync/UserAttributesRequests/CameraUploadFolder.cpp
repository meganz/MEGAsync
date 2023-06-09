#include "CameraUploadFolder.h"

#include "megaapi.h"
#include "mega/types.h"
#include "MegaApplication.h"

namespace UserAttributes
{
//CAMERA UPLOAD FOLDERS REQUEST
//
//
void CameraUploadFolder::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        bool secondary = incoming_request->getFlag();
        if(!secondary)
        {
            mCameraUploadFolderHandle = incoming_request->getNodeHandle();
            if(isCameraUploadFolderReady())
            {
                emit cameraUploadFolderReady(getCameraUploadFolderHandle());
            }
        }
        else
        {
            mCameraUploadFolderSecondaryHandle = incoming_request->getNodeHandle();
            if(isCameraUploadFolderSecondaryReady())
            {
                emit cameraUploadFolderSecondaryReady(getCameraUploadFolderSecondaryHandle());
            }
        }
    }
}

void CameraUploadFolder::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_CAMERA_UPLOADS_FOLDER);
}

AttributeRequest::RequestInfo CameraUploadFolder::fillRequestInfo()
{
    std::function<void()> cameraRequestFunc = [](){
        MegaSyncApp->getMegaApi()->getCameraUploadsFolder();
        MegaSyncApp->getMegaApi()->getCameraUploadsFolderSecondary();
    };
    QSharedPointer<ParamInfo> acameraParamInfo(new ParamInfo(cameraRequestFunc, QList<int>()<<mega::MegaError::API_OK));
    ParamInfoMap paramInfo({{mega::MegaApi::USER_ATTR_CAMERA_UPLOADS_FOLDER, acameraParamInfo}});
    RequestInfo ret(paramInfo, QMap<int64_t, int>({{mega::MegaUser::CHANGE_TYPE_CAMERA_UPLOADS_FOLDER, mega::MegaApi::USER_ATTR_CAMERA_UPLOADS_FOLDER}}));
    return ret;
}

bool CameraUploadFolder::isAttributeReady() const
{
    return mCameraUploadFolderHandle != mega::INVALID_HANDLE
           && mCameraUploadFolderSecondaryHandle != mega::INVALID_HANDLE;
}

bool CameraUploadFolder::isCameraUploadFolderReady() const
{
    return mCameraUploadFolderHandle != mega::INVALID_HANDLE;
}

bool CameraUploadFolder::isCameraUploadFolderSecondaryReady() const
{
    return mCameraUploadFolderSecondaryHandle != mega::INVALID_HANDLE;
}

const mega::MegaHandle &CameraUploadFolder::getCameraUploadFolderHandle() const
{
    return mCameraUploadFolderHandle;
}

const mega::MegaHandle &CameraUploadFolder::getCameraUploadFolderSecondaryHandle() const
{
    return mCameraUploadFolderSecondaryHandle;
}

std::shared_ptr<const CameraUploadFolder> CameraUploadFolder::requestCameraUploadFolder()
{
    return UserAttributesManager::instance().requestAttribute<CameraUploadFolder>();
}

}//end namespace UserAttributes
