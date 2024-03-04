#include "MyBackupsHandle.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"

namespace UserAttributes
{
MyBackupsHandle::MyBackupsHandle(const QString &userEmail)
 : AttributeRequest(userEmail),
   mMyBackupsFolderHandle(mega::INVALID_HANDLE),
   mMyBackupsFolderPath(QString()),
   mCreationRequested(false),
   mCreateBackupsListener(nullptr)
{
    qRegisterMetaType<mega::MegaHandle>("mega::MegaHandle");
}

std::shared_ptr<MyBackupsHandle> MyBackupsHandle::requestMyBackupsHandle()
{
    return UserAttributesManager::instance().requestAttribute<MyBackupsHandle>();
}

void MyBackupsHandle::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* error)
{
    mega::MegaHandle newValue = mega::INVALID_HANDLE;
    if(error->getErrorCode() == mega::MegaError::API_OK)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Got MyBackups folder from remote");
        newValue = incoming_request->getNodeHandle();
        mCreateBackupsListener.reset();
    }
    else if(error->getErrorCode() == mega::MegaError::API_ENOENT)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "MyBackups folder does not exist");
        if(mCreationRequested)
        {
            createMyBackupsFolderIfNeeded();
        }
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Error getting MyBackups folder: \"%1\"")
                     .arg(QString::fromUtf8(error->getErrorString()))
                     .toUtf8().constData());
        mMyBackupsFolderPath.clear();
    }

    onMyBackupsFolderReady(newValue);
}

void MyBackupsHandle::onMyBackupsFolderReady(mega::MegaHandle h)
{
    if (h != mMyBackupsFolderHandle)
    {
        mMyBackupsFolderHandle = h;
        std::unique_ptr<char[]> path (MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(h));
        mMyBackupsFolderPath = QString::fromUtf8(path.get());
    }
    emit attributeReady(mMyBackupsFolderHandle);
}

AttributeRequest::RequestInfo MyBackupsHandle::fillRequestInfo()
{
    std::function<void()> requestFunc = []()
    {
        MegaSyncApp->getMegaApi()->getUserAttribute(mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER);
    };
    QSharedPointer<ParamInfo> paramInfo(new ParamInfo(requestFunc, QList<int>()
                                                      << mega::MegaError::API_OK
                                                      << mega::MegaError::API_ENOENT));
    ParamInfoMap paramInfoMap({{mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER, paramInfo}});
    RequestInfo ret(paramInfoMap, QMap<int64_t, int>({{mega::MegaUser::CHANGE_TYPE_MY_BACKUPS_FOLDER,
                                                   mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER}}));
    return ret;
}

mega::MegaHandle MyBackupsHandle::getMyBackupsHandle() const
{
    return mMyBackupsFolderHandle;
}

void MyBackupsHandle::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER);
}

bool MyBackupsHandle::isAttributeReady() const
{
    return mMyBackupsFolderHandle != mega::INVALID_HANDLE;
}

QString MyBackupsHandle::getMyBackupsLocalizedPath()
{
    return QLatin1Char('/') + MegaNodeNames::getBackupsName();
}

QString MyBackupsHandle::getNodeLocalizedPath(QString path) const
{
// The path looks like "/Backups" (but localized), without the "/Vault" root
// Note: if the node exists, its name is ignored and we always display the localized version,
// as per requirements.
    QString localizedPath (path);
    if (mMyBackupsFolderHandle != mega::INVALID_HANDLE)
    {
        localizedPath = getMyBackupsLocalizedPath()
                + QStringRef(&path, mMyBackupsFolderPath.size(), path.size() - mMyBackupsFolderPath.size()) ;
    }
    return localizedPath;
}

void MyBackupsHandle::createMyBackupsFolderIfNeeded()
{
    if(!isAttributeReady()
            && !isAttributeRequestPending(mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER)
            && !attributeRequestNeedsRetry(mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER))
    {
        QString name = MegaNodeNames::getBackupsName();
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Creating MyBackups folder");
        if (!mCreateBackupsListener)
        {
            mCreateBackupsListener.reset(new CreateMyBackupsListener());
            connect(mCreateBackupsListener.get(), &CreateMyBackupsListener::backupFolderCreated, this, &MyBackupsHandle::onMyBackupsFolderReady);
        }
        MegaSyncApp->getMegaApi()->setMyBackupsFolder(name.toUtf8().constData(), mCreateBackupsListener.get());
        mCreationRequested = false;
    }
    else if(isAttributeRequestPending(mega::MegaApi::USER_ATTR_MY_BACKUPS_FOLDER))
    {
        mCreationRequested = true;
    }
}

CreateMyBackupsListener::CreateMyBackupsListener()
    : mDelegateListener(new mega::QTMegaRequestListener(MegaSyncApp->getMegaApi(), this))
{
}

void CreateMyBackupsListener::onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *error)
{
    if(incoming_request->getType() == mega::MegaRequest::TYPE_SET_MY_BACKUPS)
    {
        if(error->getErrorCode() == mega::MegaError::API_OK)
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "MyBackups folder created");
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               QString::fromUtf8("Error creating MyBackups folder: %1")
                               .arg(error->getErrorCode()).toUtf8().constData());
        }
        emit backupFolderCreated(incoming_request->getNodeHandle());
    }
}

CreateMyBackupsListener::~CreateMyBackupsListener()
{
    mDelegateListener->deleteLater();
}

}//end namespace UserAttributes
