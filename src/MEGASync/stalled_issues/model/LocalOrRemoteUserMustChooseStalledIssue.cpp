#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include "MegaApplication.h"
#include "MegaUploader.h"
#include "StalledIssuesUtilities.h"
#include "StatsEventHandler.h"
#include "TransfersModel.h"

#include <QRandomGenerator>

#include <optional>

LocalOrRemoteUserMustChooseStalledIssue::LocalOrRemoteUserMustChooseStalledIssue(
    const mega::MegaSyncStall* stallIssue):
    StalledIssue(stallIssue)
{
}

StalledIssue::AutoSolveIssueResult LocalOrRemoteUserMustChooseStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);

    auto sideSelector = [this](qint64 localValue, qint64 remoteValue) -> std::optional<bool>
    {
        if (localValue != remoteValue)
        {
            return std::optional<bool>(localValue > remoteValue ? chooseLocalSide() :
                                                                  chooseRemoteSide());
        }

        return std::nullopt;
    };

    // Check modified time
    auto localModifiedTimeInMSecs(consultLocalData()->getAttributes()->modifiedTimeInMSecs());
    auto cloudModifiedTimeInMSecs(consultCloudData()->getAttributes()->modifiedTimeInMSecs());

    auto result(sideSelector(localModifiedTimeInMSecs, cloudModifiedTimeInMSecs));
    if (!result.has_value())
    {
        // if the modified time was the same, check size
        auto localSize(consultLocalData()->getAttributes()->size());
        auto cloudSize(consultCloudData()->getAttributes()->size());

        result = sideSelector(localSize, cloudSize);

        if (!result.has_value())
        {
            // If the size is the same, randomly choose the local or remote side. We will still have
            // a 50% success rate.
            // We get a random number between 0 (inclusive) and 2 (exclusive) and choose the local
            // or remote side
            result = std::optional<bool>(QRandomGenerator::global()->bounded(0, 2) == 0 ?
                                             chooseLocalSide() :
                                             chooseRemoteSide());
        }
    }

    // If the result is true, the issue has been solved.
    if (result.has_value() && result.value())
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_AUTOMATICALLY);
        return StalledIssue::AutoSolveIssueResult::SOLVED;
    }

    return StalledIssue::AutoSolveIssueResult::FAILED;
}

bool LocalOrRemoteUserMustChooseStalledIssue::chooseLastMTimeSide()
{
    return lastModifiedSide() == ChosenSide::LOCAL ? chooseLocalSide() : chooseRemoteSide();
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide
    LocalOrRemoteUserMustChooseStalledIssue::lastModifiedSide() const
{
    if (isFile())
    {
        return consultLocalData()->getAttributes()->modifiedTimeInMSecs() >
                       consultCloudData()->getAttributes()->modifiedTimeInMSecs() ?
                   ChosenSide::LOCAL :
                   ChosenSide::REMOTE;
    }

    return ChosenSide::NONE;
}

bool LocalOrRemoteUserMustChooseStalledIssue::UIShowFileAttributes() const
{
    return true;
}

bool LocalOrRemoteUserMustChooseStalledIssue::isAutoSolvable() const
{
    // Only in smart mode
    auto result(false);

    if (Preferences::instance()->isStalledIssueSmartModeActivated())
    {
        // In case it is a backup, we cannot automatically solve it
        if (getSyncType() != mega::MegaSync::SyncType::TYPE_BACKUP && isFile())
        {
            if ((consultLocalData()->getAttributes()->size() !=
                 consultCloudData()->getAttributes()->size()) ||
                (consultLocalData()->getAttributes()->modifiedTimeInMSecs() !=
                 consultCloudData()->getAttributes()->modifiedTimeInMSecs()) ||
                (consultLocalData()->getAttributes()->getCRC() !=
                 consultCloudData()->getAttributes()->getCRC()))
            {
                // Check names
                auto localName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(
                    consultLocalData()->getFileName().toUtf8().constData(),
                    nullptr)));
                auto cloudName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(
                    consultCloudData()->getFileName().toUtf8().constData(),
                    nullptr)));

                if (localName.compare(cloudName, Qt::CaseSensitive) == 0)
                {
                    result = true;
                }
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
    info->state = TransferData::ACTIVE_STATES_MASK;
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
        info->state = TransferData::ACTIVE_STATES_MASK;
        //Check if transfer already exists
        if (!isBeingSolvedByUpload(info, true))
        {
            std::shared_ptr<mega::MegaNode> node(getCloudData()->getNode());
            std::shared_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(info->parentHandle));
            if(parentNode)
            {
                mChosenSide = ChosenSide::LOCAL;

                bool versionsDisabled(Preferences::instance()->fileVersioningDisabled());
                if(versionsDisabled)
                {
                    mError = Utilities::removeSyncRemoteFile(node.get());
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
    auto syncId = syncIds().isEmpty() ? mega::INVALID_HANDLE : firstSyncId();
    mChosenSide = ChosenSide::REMOTE;
    return Utilities::removeLocalFile(consultLocalData()->getNativeFilePath(), syncId);
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

std::shared_ptr<mega::MegaError> LocalOrRemoteUserMustChooseStalledIssue::getRemoveRemoteError() const
{
    return mError;
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::getChosenSide() const
{
    return mChosenSide;
}
