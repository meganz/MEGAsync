#include "StalledIssue.h"

#include "MegaApplication.h"

StalledIssueData::StalledIssueData()
    : mIsCloud(false)
{
    qRegisterMetaType<StalledIssueDataPtr>("StalledIssueDataPtr");
    qRegisterMetaType<StalledIssuesDataList>("StalledIssuesDataList");
    qRegisterMetaType<StalledIssuesList>("StalledIssuesList");
}

const StalledIssueData::Path &StalledIssueData::getPath() const
{
    return mPath;
}

const StalledIssueData::Path &StalledIssueData::getMovePath() const
{
    return mMovePath;
}

bool StalledIssueData::isCloud() const
{
    return mIsCloud;
}

bool StalledIssueData::hasMoveInfo() const
{
    return !mMovePath.isEmpty();
}

bool StalledIssueData::isEmpty() const
{
    return mPath.path.isEmpty() && !hasMoveInfo();
}

QString StalledIssueData::getFilePath() const
{
    QFileInfo filePath(mPath.path);
    return filePath.filePath();
}

QString StalledIssueData::getMoveFilePath() const
{
    QFileInfo filePath(mMovePath.path);
    return filePath.filePath();
}

QString StalledIssueData::getNativeFilePath() const
{
    if(isCloud())
    {
        return mPath.path;
    }
    else
    {
        QFileInfo filePath(mPath.path);
        return QDir::toNativeSeparators(filePath.filePath());
    }
}

QString StalledIssueData::getNativeMoveFilePath() const
{
    if(isCloud())
    {
        return mMovePath.path;
    }
    else
    {
        QFileInfo filePath(mMovePath.path);
        return QDir::toNativeSeparators(filePath.filePath());
    }
}

QString StalledIssueData::getNativePath() const
{
    QFileInfo filePath(mPath.path);
    return QDir::toNativeSeparators(filePath.path());
}

QString StalledIssueData::getNativeMovePath() const
{
    QFileInfo filePath(mMovePath.path);
    return QDir::toNativeSeparators(filePath.path());
}

QString StalledIssueData::getFileName() const
{
    QFileInfo filePath(getNativeFilePath());

    if(filePath.isFile())
    {
        return filePath.fileName();
    }

    if(isCloud())
    {
        auto splittedIndexPath = mPath.path.split(QString::fromUtf8("/"));
        return splittedIndexPath.last();
    }
    else
    {
        auto splittedIndexPath = mPath.path.split(QString::fromUtf8("\\"));
        return splittedIndexPath.last();
    }
}

//Conflicted Names stalled issue
ConflictedNamesStalledIssue::ConflictedNamesStalledIssue()
    : StalledIssue()
{
    mIsNameConflict = true;
}

ConflictedNamesStalledIssue::ConflictedNamesStalledIssue(mega::MegaSyncNameConflict *nameConflictStallIssue)
    : StalledIssue()
{
    mIsNameConflict = true;
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
            initCloudIssue();

            getCloudData()->mPath.path = QString::fromUtf8(cp);
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
            initLocalIssue();
            getLocalData()->mPath.path = QString::fromUtf8(lp);
        }
    }

    mIsNameConflict = true;
}

StalledIssue::StalledIssue(const mega::MegaSyncStall *stallIssue)
    : mReason(stallIssue->reason()),
      mIsImmediate(stallIssue->isImmediate()),
      mReasonString(QString::fromStdString(stallIssue->reasonString())),
      mIsCloud(stallIssue->isCloud())
{
    fillIssue(stallIssue);
}

bool StalledIssue::initLocalIssue()
{
    if(!mLocalData)
    {
        mLocalData = StalledIssueDataPtr(new StalledIssueData());
        return true;
    }

    return false;
}

bool StalledIssue::initCloudIssue()
{
    if(!mCloudData)
    {
        mCloudData = StalledIssueDataPtr(new StalledIssueData());
        mCloudData->mIsCloud = true;

        return true;
    }

    return false;
}

void StalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    if(stall->isCloud())
    {
        initCloudIssue();

        getCloudData()->mPath.path = QString::fromUtf8(stall->indexPath());

        QString cloudPath(QString::fromUtf8(stall->cloudPath()));
        if(!cloudPath.isEmpty())
        {
            QFileInfo cloudPathInfo(cloudPath);
            getCloudData()->mMovePath.path = cloudPathInfo.isFile() ? cloudPathInfo.path() : cloudPathInfo.filePath();
        }

        QString localPath(QString::fromUtf8(stall->localPath()));
        if(!localPath.isEmpty() && initLocalIssue())
        {
            getLocalData()->mPath.path = QString::fromUtf8(stall->localPath());
        }
    }
    else
    {
        initLocalIssue();

        getLocalData()->mPath.path = QString::fromUtf8(stall->indexPath());

        QString localPath(QString::fromUtf8(stall->localPath()));
        if(!localPath.isEmpty())
        {
            QFileInfo localPathInfo(localPath);
            getLocalData()->mMovePath.path = QDir::toNativeSeparators(localPathInfo.path());
        }

        QString cloudPath(QString::fromUtf8(stall->cloudPath()));
        if(!cloudPath.isEmpty() && initCloudIssue())
        {
            QFileInfo cloudPathInfo(cloudPath);
            getCloudData()->mPath.path = cloudPathInfo.path();
        }
    }
}

const QExplicitlySharedDataPointer<StalledIssueData>& StalledIssue::getLocalData() const
{
    return mLocalData;
}

const QExplicitlySharedDataPointer<StalledIssueData>& StalledIssue::getCloudData() const
{
    return mCloudData;
}

mega::MegaSyncStall::SyncStallReason StalledIssue::getReason() const
{
    return mReason;
}

QString StalledIssue::getFileName() const
{
    if(mLocalData)
    {
        return mLocalData->getFileName();
    }
    else if(mCloudData)
    {
        return mCloudData->getFileName();
    }
    else
    {
        return QString();
    }
}

bool StalledIssue::isCloud() const
{
    return mIsCloud;
}

bool StalledIssue::isNameConflict() const
{
    return mIsNameConflict;
}

bool StalledIssue::operator==(const StalledIssue &data)
{   
    bool equal(true);

    equal &= (mLocalData == data.getLocalData());
    equal &= (mCloudData == data.getCloudData());

    return equal;
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
