#include "StalledIssue.h"

#include "MegaApplication.h"

StalledIssueData::StalledIssueData(const mega::MegaSyncStall *stallIssue)
    : mIsCloud(false)
    , mIsImmediate(false)
{
    update(stallIssue);

    qRegisterMetaType<StalledIssueDataPtr>("StalledIssueDataPtr");
    qRegisterMetaType<StalledIssuesDataList>("StalledIssuesDataList");
    qRegisterMetaType<StalledIssuesList>("StalledIssuesList");
}

void StalledIssueData::update(const mega::MegaSyncStall *stallIssue)
{
    if(stallIssue)
    {
        mIsCloud      = stallIssue->isCloud();
        mIsImmediate  = stallIssue->isImmediate();
        mReasonString = QString::fromUtf8(stallIssue->reasonString());
    }
}

bool StalledIssueData::hasMoveInfo() const
{
    return !mMovePath.isEmpty();
}

//Conflicted Names stalled issue
ConflictedNamesStalledIssue::ConflictedNamesStalledIssue()
    : StalledIssue()
{
    mIsNameConflict = true;
}

ConflictedNamesStalledIssue::ConflictedNamesStalledIssue(const QExplicitlySharedDataPointer<StalledIssueData> &tdr)
{
    mIsNameConflict = true;
    d.append(tdr);
}


ConflictedNamesStalledIssue::ConflictedNamesStalledIssue(mega::MegaSyncNameConflict *nameConflictStallIssue)
    : StalledIssue()
{
    mIsNameConflict = true;
    auto data = StalledIssueDataPtr(new StalledIssueData());
    d.append(data);
    update(nameConflictStallIssue);
}

QStringList ConflictedNamesStalledIssue::localNames() const
{
    return mLocalNames;
}

QStringList ConflictedNamesStalledIssue::cloudNames() const
{
    return mCloudNames;
}

void ConflictedNamesStalledIssue::update(const mega::MegaSyncNameConflict* nameConflictStallIssue)
{
    if (mega::MegaStringList* cn = nameConflictStallIssue->cloudNames())
    {
        for (int j = 0; j < cn->size(); ++j)
        {
            mCloudNames.append(QString::fromUtf8(cn->get(j)));
        }
    }
    if (auto cp = nameConflictStallIssue->cloudPath())
    {
        if (cp && *cp)
        {
            getStalledIssueData()->mPath.path = QString::fromUtf8(cp);
        }
    }
    if (mega::MegaStringList* ln = nameConflictStallIssue->localNames())
    {
        for (int j = 0; j < ln->size(); ++j)
        {
            mLocalNames.append(QString::fromUtf8(ln->get(j)));
        }
    }
    if (auto lp = nameConflictStallIssue->localPath())
    {
        if (lp && *lp)
        {
            getStalledIssueData()->mPath.path = QString::fromUtf8(lp);
        }
    }

    mIsNameConflict = true;
}

StalledIssue::StalledIssue(const mega::MegaSyncStall *stallIssue)
{
    extractFileName(stallIssue);
    fillIssue(stallIssue);
    mReason = stallIssue->reason();
}

void StalledIssue::addStalledIssueData(const mega::MegaSyncStall *stallIssue)
{
    fillIssue(stallIssue);
}

void StalledIssue::addStalledIssueData(QExplicitlySharedDataPointer<StalledIssueData> data)
{
    d.append(data);
}

void StalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssueDataPtr tdr(new StalledIssueData(stall));
    d.append(tdr);

    if(stall->isCloud())
    {
        tdr->mPath.path    = QString::fromUtf8(stall->indexPath());
    }
    else
    {
        QFileInfo pathInfo(QString::fromUtf8(stall->indexPath()));
        tdr->mPath.path = QDir::toNativeSeparators(pathInfo.path());
    }

    if(stall->isCloud())
    {
        QString cloudPath(QString::fromUtf8(stall->cloudPath()));
        if(!cloudPath.isEmpty())
        {
            QFileInfo cloudPathInfo(cloudPath);
            getStalledIssueData()->mMovePath.path = cloudPathInfo.isFile() ? cloudPathInfo.path() : cloudPathInfo.filePath();
        }

        QString localPath(QString::fromUtf8(stall->localPath()));
        if(!localPath.isEmpty())
        {
            auto localData = StalledIssueDataPtr(new StalledIssueData());
            addStalledIssueData(localData);

            QFileInfo localPathInfo(localPath);
            localData->mPath.path = localPathInfo.path();
        }
    }
    else
    {
        QString localPath(QString::fromUtf8(stall->localPath()));
        if(!localPath.isEmpty())
        {
            QFileInfo localPathInfo(localPath);
            getStalledIssueData()->mMovePath.path = localPathInfo.path();
        }

        QString cloudPath(QString::fromUtf8(stall->cloudPath()));
        if(!cloudPath.isEmpty())
        {
            auto cloudData = StalledIssueDataPtr(new StalledIssueData());
            addStalledIssueData(cloudData);

            QFileInfo cloudPathInfo(cloudPath);
            cloudData->mPath.path = cloudPathInfo.path();
        }
    }
}

void StalledIssue::extractFileName(const mega::MegaSyncStall *stall)
{
    QFileInfo fileInfo(QString::fromUtf8(stall->indexPath()));
    mFileName = fileInfo.fileName();
}

QExplicitlySharedDataPointer<StalledIssueData> StalledIssue::getStalledIssueData(int index) const
{
    return d.size() > index ? d.at(index) : QExplicitlySharedDataPointer<StalledIssueData>();
}

int StalledIssue::stalledIssuesCount() const
{
    return d.size();
}

mega::MegaSyncStall::SyncStallReason StalledIssue::getReason() const
{
    return mReason;
}

const QString& StalledIssue::getFileName() const
{
    return mFileName;
}

bool StalledIssue::isCloud() const
{
    if(stalledIssuesCount() > 0)
    {
        return getStalledIssueData()->mIsCloud;
    }

    return false;
}

bool StalledIssue::isNameConflict() const
{
    return mIsNameConflict;
}

bool StalledIssue::operator==(const StalledIssue &data)
{
    if(data.stalledIssuesCount() != stalledIssuesCount())
    {
        return false;
    }

    for(int index = 0; index < stalledIssuesCount(); ++index)
    {
       if(getStalledIssueData(index) != data.getStalledIssueData(index))
       {
           return false;
       }
    }

    return true;
}

StalledIssueFilterCriterion StalledIssue::getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason)
{
    switch (reason)
    {
        case mega::MegaSyncStall::SyncStallReason::ApplyMoveNeedsOtherSideParentFolderToExist:
        case mega::MegaSyncStall::SyncStallReason::ApplyMoveIsBlockedByExistingItem:
        case mega::MegaSyncStall::SyncStallReason::MoveNeedsDestinationNodeProcessing:
        case mega::MegaSyncStall::SyncStallReason::UpsyncNeedsTargetFolder:
        case mega::MegaSyncStall::SyncStallReason::DownsyncNeedsTargetFolder:
        case mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning:
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        case mega::MegaSyncStall::SyncStallReason::WaitingForFileToStopChanging:
        case mega::MegaSyncStall::SyncStallReason::MovingDownloadToTarget:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::CouldNotMoveToLocalDebrisFolder:
        case mega::MegaSyncStall::SyncStallReason::LocalFolderNotScannable:
        case mega::MegaSyncStall::SyncStallReason::SymlinksNotSupported:
        case mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile:
        case mega::MegaSyncStall::SyncStallReason::MatchedAgainstUnidentifiedItem:
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameFailed:
        {
            return StalledIssueFilterCriterion::ITEM_TYPE_CONFLICTS;
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CreateFolderFailed:
        case mega::MegaSyncStall::SyncStallReason::UnknownExclusionState:
        case mega::MegaSyncStall::SyncStallReason::UnableToLoadIgnoreFile:
        case mega::MegaSyncStall::SyncStallReason::MoveTargetNameTooLong:
        case mega::MegaSyncStall::SyncStallReason::DownloadTargetNameTooLong:
        case mega::MegaSyncStall::SyncStallReason::CreateFolderNameTooLong:
        case mega::MegaSyncStall::SyncStallReason::CantFingrprintFileYet:
        case mega::MegaSyncStall::SyncStallReason::FolderContainsLockedFiles:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        case mega::MegaSyncStall::SyncStallReason::MACVerificationFailure:
        case mega::MegaSyncStall::SyncStallReason::NoNameTripletsDetected:
        case mega::MegaSyncStall::SyncStallReason::EncounteredHardLinkAtMoveSource:
        case mega::MegaSyncStall::SyncStallReason::SpecialFilesNotSupported:
        default:
        {
            return StalledIssueFilterCriterion::OTHER_CONFLICTS;
            break;
        }
    }
}
