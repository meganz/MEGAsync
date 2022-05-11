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
    : mReason(stallIssue->reason())
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
    auto localSourcePathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(false,0));
    auto localTargetPathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(false,1));

    auto localSourcePath = QString::fromUtf8(stall->path(false,0));
    auto localTargetPath = QString::fromUtf8(stall->path(false,1));

    if(localSourcePathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !localSourcePath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mPath.path = localSourcePath;
        getLocalData()->mPath.mPathProblem = localSourcePathProblem;
    }

    if(localTargetPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !localTargetPath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mMovePath.path = localTargetPath;
        getLocalData()->mMovePath.mPathProblem = localTargetPathProblem;
    }

    auto cloudSourcePathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(true,0));
    auto cloudTargetPathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(true,1));

    auto cloudSourcePath = QString::fromUtf8(stall->path(true,0));
    auto cloudTargetPath = QString::fromUtf8(stall->path(true,1));

    if(cloudSourcePathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !cloudSourcePath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mPath.path = cloudSourcePath;
        getCloudData()->mPath.mPathProblem = cloudSourcePathProblem;
    }

    if(cloudTargetPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !cloudTargetPath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mMovePath.path = cloudTargetPath;
        getCloudData()->mMovePath.mPathProblem = cloudTargetPathProblem;
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
        case mega::MegaSyncStall::SyncStallReason::FileIssue:
        {
            return StalledIssueFilterCriterion::ITEM_TYPE_CONFLICTS;
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur:
        case mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning:
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        case mega::MegaSyncStall::SyncStallReason::UploadIssue:
        case mega::MegaSyncStall::SyncStallReason::DownloadIssue:
        case mega::MegaSyncStall::SyncStallReason::CannotCreateFolder:
        case mega::MegaSyncStall::SyncStallReason::CannotPerformDeletion:
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        case mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        default:
        {
            return StalledIssueFilterCriterion::OTHER_CONFLICTS;
            break;
        }
    }
}
