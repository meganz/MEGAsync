#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include <MegaApplication.h>
#include <MegaUploader.h>
#include <TransfersModel.h>
#include <StalledIssuesUtilities.h>
#include "StatsEventHandler.h"
#include <MegaApiSynchronizedRequest.h>
#include <FileFolderAttributes.h>

LocalOrRemoteUserMustChooseStalledIssue::LocalOrRemoteUserMustChooseStalledIssue(
    const mega::MegaSyncStall* stallIssue):
    StalledIssue(stallIssue)
{
}

StalledIssue::AutoSolveIssueResult LocalOrRemoteUserMustChooseStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);
    if(chooseLastMTimeSide())
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_AUTOMATICALLY);
        return StalledIssue::AutoSolveIssueResult::SOLVED;
    }

    return StalledIssue::AutoSolveIssueResult::FAILED;
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseLastMTimeSide()
{
    if(consultLocalData()->getAttributes()->modifiedTimeInSecs() >= consultCloudData()->getAttributes()->modifiedTimeInSecs())
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
    QString localCRC;
    QString remoteCRC;

    getLocalData()->getAttributes()->requestCRC(this, [&localCRC](const QString& crc){
        localCRC = crc;
    });

    getCloudData()->getAttributes()->requestCRC(this, [&remoteCRC](const QString& crc){
        remoteCRC = crc;
    });

    if(localCRC.compare(mLocalCRCAtStart) != 0 || remoteCRC.compare(mRemoteCRCAtStart) != 0)
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
    if (isBeingSolvedByUpload(info, true))
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
        getLocalData()->getAttributes()->initAllAttributes();
        getCloudData()->getAttributes()->initAllAttributes();

        getLocalData()->getAttributes()->requestCRC(this, [this](const QString& crc){
            mLocalCRCAtStart = crc;
            });

        getCloudData()->getAttributes()->requestCRC(this, [this](const QString& crc){
            mRemoteCRCAtStart = crc;
        });
    }
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseLocalSide()
{
    if(getCloudData())
    {
        std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
        //Check if transfer already exists
        if (!isBeingSolvedByUpload(info, true))
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
                        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Unable to remove file: %1. Error: %2")
                                                                   .arg(QString::fromUtf8(node->getName()), Utilities::getTranslatedError(mError.get()))
                                                                   .toUtf8().constData());
                        return false;
                    }
                }
                else
                {
                    //Only upload the file if the versions are enabled
                    //Using appDataId == 0 means that there will be no notification for this upload
                    StalledIssuesUtilities::getMegaUploader()->upload(info->localPath,
                                                                      info->filename,
                                                                      parentNode,
                                                                      0,
                                                                      nullptr);
                }

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

bool LocalOrRemoteUserMustChooseStalledIssue::chooseBothSides()
{
    mChosenSide = ChosenSide::BOTH;
    auto result = StalledIssuesUtilities::KeepBothSides(getCloudData()->getNode(), getLocalData()->getNativeFilePath());
    if(result.sideRenamed == StalledIssuesUtilities::KeepBothSidesState::Side::LOCAL)
    {
        getLocalData()->setRenamedFileName(result.newName);
    }
    else if(result.sideRenamed == StalledIssuesUtilities::KeepBothSidesState::Side::REMOTE)
    {
        getCloudData()->setRenamedFileName(result.newName);
    }

    return result.error == nullptr;
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::lastModifiedSide() const
{
    if(isFile())
    {
        return consultLocalData()->getAttributes()->modifiedTimeInSecs() > consultCloudData()->getAttributes()->modifiedTimeInSecs()
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
