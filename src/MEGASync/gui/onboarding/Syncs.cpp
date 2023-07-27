#include "Syncs.h"
#include "syncs/control/SyncController.h"
#include "MegaApplication.h"
#include "TextDecorator.h"
#include "QMegaMessageBox.h"

Syncs::Syncs(QObject *parent)
    : QObject(parent)
    , mSyncController(new SyncController())
{
    connect(mSyncController, &SyncController::syncAddStatus,
            this, &Syncs::onSyncAddRequestStatus);
}

Syncs::~Syncs()
{
    delete mSyncController;
}

void Syncs::addSync(const QString &localPath, mega::MegaHandle remoteHandle)
{
    if(remoteHandle == mega::INVALID_HANDLE)
    {
        remoteHandle = MegaSyncApp->getRootNode()->getHandle();
    }

    QString warningMessage;
    auto syncability (SyncController::isLocalFolderAllowedForSync(localPath, mega::MegaSync::TYPE_TWOWAY, warningMessage));
    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = SyncController::areLocalFolderAccessRightsOk(localPath, mega::MegaSync::TYPE_TWOWAY, warningMessage);
    }

    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = SyncController::isLocalFolderSyncable(localPath, mega::MegaSync::TYPE_TWOWAY, warningMessage);
    }

    // If OK, check that we can sync the selected remote folder
    std::shared_ptr<mega::MegaNode> node (MegaSyncApp->getMegaApi()->getNodeByHandle(remoteHandle));

    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = std::max(SyncController::isRemoteFolderSyncable(node, warningMessage), syncability);
    }

    if (syncability == SyncController::CANT_SYNC)
    {
        // If can't sync because remote node does not exist, try to create it
        if (!node)
        {
            auto rootNode = MegaSyncApp->getRootNode();
            if (!rootNode)
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = QMegaMessageBox::errorTitle();
                msgInfo.text =  tr("Unable to get the filesystem.\n"
                                   "Please, try again. If the problem persists "
                                   "please contact bug@mega.co.nz");

                QMegaMessageBox::warning(msgInfo);
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !rootNode (Onboarding)");
                Preferences::instance()->setCrashed(true);
                MegaSyncApp->rebootApplication(false);
            }
        }
        else
        {
            emit cantSync(warningMessage);
        }
    }
    else if (syncability == SyncController::CAN_SYNC
               || syncability == SyncController::WARN_SYNC)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.text =  warningMessage
                       + QLatin1Char('\n')
                       + tr("Do you want to continue?");
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No, QMessageBox::No;
        msgInfo.finishFunc = [this, localPath, remoteHandle](QPointer<QMessageBox> msgBox){
            if(msgBox->result() == QMessageBox::Yes)
            {
                mSyncController->addSync(localPath, remoteHandle);
            }
        };

        QMegaMessageBox::warning(msgInfo);
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
    }
    else
    {
        emit syncSetupSuccess();
    }
}
