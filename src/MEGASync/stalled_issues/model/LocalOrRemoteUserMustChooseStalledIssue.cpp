#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include <MegaApplication.h>
#include <MegaUploader.h>
#include <TransfersModel.h>
#include <StalledIssuesUtilities.h>
#include "StatsEventHandler.h"
#include <MegaApiSynchronizedRequest.h>
#include <FileFolderAttributes.h>


LocalOrRemoteUserMustChooseStalledIssue::LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue),
      mUploader(new MegaUploader(MegaSyncApp->getMegaApi(), nullptr))
{
}

LocalOrRemoteUserMustChooseStalledIssue::~LocalOrRemoteUserMustChooseStalledIssue()
{
    mUploader->deleteLater();
}

bool LocalOrRemoteUserMustChooseStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);
    if(chooseLastMTimeSide())
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_AUTOMATICALLY);
        return true;
    }

    return false;
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseLastMTimeSide()
{
    if(consultLocalData()->getAttributes()->modifiedTime() >= consultCloudData()->getAttributes()->modifiedTime())
    {
        return chooseLocalSide();
    }
    else
    {
        return chooseRemoteSide();
    }
}

bool LocalOrRemoteUserMustChooseStalledIssue::UIShowFileAttributes() const
{
    return true;
}

bool LocalOrRemoteUserMustChooseStalledIssue::isAutoSolvable() const
{
    //Only in smart mode
    auto result(false);

    if(Preferences::instance()->isStalledIssueSmartModeActivated())
    {
        //In case it is a backup, we cannot automatically solve it
        if(getSyncType() == mega::MegaSync::SyncType::TYPE_BACKUP)
        {
            return false;
        }

        if(isFile() && (consultLocalData()->getAttributes()->size() == consultCloudData()->getAttributes()->size()))
        {
            //Check names
            auto localName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultLocalData()->getFileName().toUtf8().constData())));
            auto cloudName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultCloudData()->getFileName().toUtf8().constData())));
            if(localName.compare(cloudName, Qt::CaseSensitive) == 0)
            {
                result = true;
            }
        }
    }

    return result;
}

void LocalOrRemoteUserMustChooseStalledIssue::setIsSolved(SolveType type)
{
    StalledIssue::setIsSolved(type);
    if(isSolved())
    {
        mError.reset();
    }
}

bool LocalOrRemoteUserMustChooseStalledIssue::checkForExternalChanges()
{
    QString localFingerprint;
    QString remoteFingerprint;

    getLocalData()->getAttributes()->requestCRC(this, [&localFingerprint](const QString& fp){
        localFingerprint = fp;
    });

    getCloudData()->getAttributes()->requestCRC(this, [&remoteFingerprint](const QString& fp){
        remoteFingerprint = fp;
    });

    if(localFingerprint.compare(mLocalFingerprintAtStart) != 0 || remoteFingerprint.compare(mRemoteFingerprintAtStart) != 0)
    {
        return true;
    }

    return false;
}

void LocalOrRemoteUserMustChooseStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssue::fillIssue(stall);

    std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
    //Check if transfer already exists
    if(isBeingSolvedByUpload(info))
    {
        setIsSolved(StalledIssue::SolveType::SOLVED);
    }

}

void LocalOrRemoteUserMustChooseStalledIssue::endFillingIssue()
{
    StalledIssue::endFillingIssue();

    if(isFile())
    {
        //For autosolving
        getLocalData()->getAttributes()->requestSize(nullptr, nullptr);
        getCloudData()->getAttributes()->requestSize(nullptr, nullptr);

        getLocalData()->getAttributes()->requestModifiedTime(nullptr, nullptr);
        getCloudData()->getAttributes()->requestModifiedTime(nullptr, nullptr);

        getLocalData()->getAttributes()->requestCRC(this, [this](const QString& fp){
            mLocalFingerprintAtStart = fp;
            });

        getCloudData()->getAttributes()->requestCRC(this, [this](const QString& fp){
            mRemoteFingerprintAtStart = fp;
        });
    }
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseLocalSide()
{
    if(getCloudData())
    {
        std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
        //Check if transfer already exists
        if(!isBeingSolvedByUpload(info))
        {
            std::shared_ptr<mega::MegaNode> node(getCloudData()->getNode());
            std::shared_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(info->parentHandle));
            if(parentNode)
            {
                mChosenSide = ChosenSide::LOCAL;

                bool versionsDisabled(Preferences::instance()->fileVersioningDisabled());
                StalledIssuesUtilities utilities;
                if(versionsDisabled)
                {
                    mError = utilities.removeRemoteFile(node.get());
                    if(mError)
                    {
                        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Unable to rename file: %1. Error: %2")
                                                                   .arg(QString::fromUtf8(node->getName()), Utilities::getTranslatedError(mError.get()))
                                                                   .toUtf8().constData());
                        return false;
                    }
                }

                //Using appDataId == 0 means that there will be no notification for this upload
                mUploader->upload(info->localPath, info->filename, parentNode, 0, nullptr);

                return true;
            }
        }
    }

    return false;
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseRemoteSide()
{
    StalledIssuesUtilities utilities;
    auto syncId = syncIds().isEmpty() ? mega::INVALID_HANDLE : firstSyncId();
    mChosenSide = ChosenSide::REMOTE;
    return utilities.removeLocalFile(consultLocalData()->getNativeFilePath(), syncId);
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseBothSides(QStringList* namesUsed)
{
    auto result(false);
    auto node(getCloudData()->getNode());
    if(node)
    {
        std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getParentNode(node.get()));
        if(parentNode)
        {
            mNewName = Utilities::getNonDuplicatedNodeName(node.get(), parentNode.get(), QString::fromUtf8(node->getName()), true, (*namesUsed));
            namesUsed->append(mNewName);

            auto error = MegaApiSynchronizedRequest::runRequest(&mega::MegaApi::renameNode,
                              MegaSyncApp->getMegaApi(),
                              node.get(),
                              mNewName.toUtf8().constData());

            if(error)
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Unable to rename file: %1. Error: %2")
                                                                       .arg(QString::fromUtf8(node->getName()), Utilities::getTranslatedError(error.get()))
                                                                       .toUtf8().constData());

                QFileInfo currentFile(getLocalData()->getNativeFilePath());
                QFile file(currentFile.filePath());
                if(file.exists())
                {
                    mNewName = Utilities::getNonDuplicatedLocalName(currentFile, true, (*namesUsed));
                    currentFile.setFile(currentFile.path(), mNewName);
                    if(file.rename(QDir::toNativeSeparators(currentFile.filePath())))
                    {
                        getLocalData()->setRenamedFileName(mNewName);
                        result = true;
                    }
                }
            }
            else
            {
                getLocalData()->setRenamedFileName(mNewName);
                result = true;
            }

            mChosenSide = ChosenSide::BOTH;
            return result;
        }
    }

    return result;
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::lastModifiedSide() const
{
    if(isFile())
    {
        return consultLocalData()->getAttributes()->modifiedTime() > consultCloudData()->getAttributes()->modifiedTime()
                   ? ChosenSide::LOCAL : ChosenSide::REMOTE;
    }

    return ChosenSide::NONE;
}

std::shared_ptr<mega::MegaError> LocalOrRemoteUserMustChooseStalledIssue::getRemoveRemoteError() const
{
    return mError;
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::getChosenSide() const
{
    return mChosenSide;
}
