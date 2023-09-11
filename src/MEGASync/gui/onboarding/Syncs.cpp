#include "Syncs.h"
#include "mega/types.h"
#include "MegaApplication.h"
#include "TextDecorator.h"
#include "QMegaMessageBox.h"

Syncs::Syncs(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
    , mSyncController(new SyncController())
    , mCreatingDefaultFolder(false)
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    connect(mSyncController, &SyncController::syncAddStatus,
            this, &Syncs::onSyncAddRequestStatus);
}

Syncs::~Syncs()
{
    delete mSyncController;
}

void Syncs::addSync(ChooseLocalFolder* local, ChooseRemoteFolder* remote)
{
    // First, process local folder
    if(!processLocal(local))
    {
        return;
    }

    // Then, get the remote handle and process remote folder
    mega::MegaHandle remoteHandle = mega::INVALID_HANDLE;
    if(!remote)
    {
        // Full sync
        remoteHandle = MegaSyncApp->getRootNode()->getHandle();
    }
    else
    {
        remoteHandle = remote->getHandle();
        if(remoteHandle == mega::INVALID_HANDLE)
        {
            // Relative sync with default folder (MEGA)
            QString defaultFolder(ChooseLocalFolder::DEFAULT_FOLDER);
            defaultFolder.remove(0, 1);
            mMegaApi->createFolder(defaultFolder.toStdString().c_str(),
                                   MegaSyncApp->getRootNode().get());
            mCreatingDefaultFolder = true;
            return;
        }
    }

    // If OK, check that we can sync the selected remote folder
    processRemote(remoteHandle);
}

bool Syncs::processLocal(ChooseLocalFolder* local)
{
    // If folder is empty, the default one (MEGA) should be created
    // Otherwise, the selected folder in the ChooseLocalFolder is used
    if(!local->createDefault())
    {
        return false;
    }
    mProcessInfo.localPath = local->getFolder();

    mProcessInfo.localSyncability =
        SyncController::isLocalFolderSyncable(mProcessInfo.localPath,
                                              mega::MegaSync::TYPE_TWOWAY,
                                              mProcessInfo.localWarningMsg);

    if(mProcessInfo.localSyncability == SyncController::CANT_SYNC)
    {
        emit cantSync(mProcessInfo.localWarningMsg);
    }

    return true;
}

void Syncs::processRemote(mega::MegaHandle remoteHandle)
{
    std::shared_ptr<mega::MegaNode> node(mMegaApi->getNodeByHandle(remoteHandle));
    if(node == nullptr)
    {
        emit cantSync(tr("Folder can't be synced as it can't be located. "
                         "It may have been moved or deleted, or you might not have access."),
                      false);
        return;
    }

    QString remoteWarningMsg;
    SyncController::Syncability remoteSyncability =
        SyncController::isRemoteFolderSyncable(node, remoteWarningMsg);
    if (remoteSyncability == SyncController::CANT_SYNC)
    {
        // Only remote CANT_SYNC at this point
        // The local CANT_SYNC was processed in the processLocal method
        if (!node)
        {
            auto rootNode = MegaSyncApp->getRootNode();
            if (!rootNode)
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = QMegaMessageBox::errorTitle();
                msgInfo.text = tr("Unable to get the filesystem.\n"
                                  "Please, try again. If the problem persists "
                                  "please contact bug@mega.co.nz");

                QMegaMessageBox::warning(msgInfo);
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                   "Setting isCrashed true: !rootNode (Onboarding)");
                Preferences::instance()->setCrashed(true);
                MegaSyncApp->rebootApplication(false);
            }
        }
        else
        {
            emit cantSync(remoteWarningMsg, false);
        }
    }
    else if (mProcessInfo.localSyncability == SyncController::WARN_SYNC)
    {
        // Only local WARN_SYNC at this point
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.text =  mProcessInfo.localWarningMsg
                       + QLatin1Char('\n')
                       + tr("Do you want to continue?");
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        msgInfo.finishFunc = [this, remoteHandle](QPointer<QMessageBox> msgBox) {
            if(msgBox->result() == QMessageBox::Yes)
            {
                mSyncController->addSync(mProcessInfo.localPath, remoteHandle);
            }
            else if(msgBox->result() == QMessageBox::No)
            {
                emit cancelSync();
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
    else if (mProcessInfo.localSyncability == SyncController::CAN_SYNC
                && remoteSyncability == SyncController::CAN_SYNC)
    {
        mSyncController->addSync(mProcessInfo.localPath, remoteHandle);
    }
}

void Syncs::onRequestFinish(mega::MegaApi* api,
                            mega::MegaRequest* request,
                            mega::MegaError* error)
{
    Q_UNUSED(api)
    switch(request->getType())
    {
        case mega::MegaRequest::TYPE_CREATE_FOLDER:
        {
            QString defaultFolder(ChooseLocalFolder::DEFAULT_FOLDER);
            defaultFolder.remove(0, 1);
            if (!mCreatingDefaultFolder || defaultFolder.compare(QString::fromUtf8(request->getName())))
            {
                break;
            }

            mCreatingDefaultFolder = false;

            if (error->getErrorCode() == mega::MegaError::API_OK)
            {
                mega::MegaNode* node =
                    mMegaApi->getNodeByPath(ChooseLocalFolder::DEFAULT_FOLDER.toStdString().c_str());
                if (!node)
                {
                    emit cantSync(tr("MEGA folder doesn't exist"), false);
                }
                else
                {
                    processRemote(request->getNodeHandle());
                    delete node;
                }
            }
            else if (error->getErrorCode() != mega::MegaError::API_ESSL
                    && error->getErrorCode() != mega::MegaError::API_ESID)
            {
                emit cantSync(QCoreApplication::translate("MegaError", error->getErrorString()), false);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void Syncs::onSyncAddRequestStatus(int errorCode, const QString &errorMsg, const QString &name)
{
    Q_UNUSED(name)
    if (errorCode != mega::MegaError::API_OK)
    {
        Text::Link link(Utilities::SUPPORT_URL);
        Text::Decorator dec(&link);
        QString msg = errorMsg;
        dec.process(msg);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = tr("Error adding sync");
        msgInfo.text = msg;
        msgInfo.textFormat = Qt::RichText;

        QMegaMessageBox::warning(msgInfo);
        emit cantSync();
    }
    else
    {
        emit syncSetupSuccess();
    }
}
