#include "StalledIssue.h"

#include "MegaApplication.h"

StalledIssueData::StalledIssueData()
    : mIsCloud(false)
    , mIsSolved(false)
{
    qRegisterMetaType<StalledIssueDataPtr>("StalledIssueDataPtr");
    qRegisterMetaType<StalledIssuesDataList>("StalledIssuesDataList");

    qRegisterMetaType<StalledIssue>("StalledIssue");
    qRegisterMetaType<NameConflictedStalledIssue>("NameConflictedStalledIssue");

    qRegisterMetaType<StalledIssueVariant>("StalledIssueVariant");
    qRegisterMetaType<StalledIssuesVariantList>("StalledIssuesVariantList");
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

    if(isCloud())
    {
        return filePath.path();
    }
    else
    {
        return QDir::toNativeSeparators(filePath.path());
    }
}

QString StalledIssueData::getNativeMovePath() const
{
    QFileInfo filePath(mMovePath.path);

    if(isCloud())
    {
        return filePath.path();
    }
    else
    {
        return QDir::toNativeSeparators(filePath.path());
    }
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

bool StalledIssueData::isEqual(const mega::MegaSyncStall* stall) const
{
    QString sourcePath;
    QString targetPath;

    if(mIsCloud)
    {
        sourcePath = QString::fromUtf8(stall->path(true,0));
        targetPath = QString::fromUtf8(stall->path(true,1));
    }
    else
    {
        sourcePath = QString::fromUtf8(stall->path(false,0));
        targetPath = QString::fromUtf8(stall->path(false,1));
    }

    return (sourcePath.compare(mPath.path) == 0 || targetPath.compare(mMovePath.path) == 0);
}

bool StalledIssueData::isSolved() const
{
    return mIsSolved;
}

void StalledIssueData::setIsSolved(bool newIsSolved)
{
    mIsSolved = newIsSolved;
}

StalledIssue::StalledIssue(const mega::MegaSyncStall *stallIssue)
{
    fillIssue(stallIssue);
}

bool StalledIssue::initLocalIssue()
{
    if(!mLocalData)
    {
        mLocalData = QExplicitlySharedDataPointer<StalledIssueData>(new StalledIssueData());
        return true;
    }

    return false;
}

bool StalledIssue::initCloudIssue()
{
    if(!mCloudData)
    {
        mCloudData = QExplicitlySharedDataPointer<StalledIssueData>(new StalledIssueData());
        mCloudData->mIsCloud = true;

        return true;
    }

    return false;
}

void StalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    mReason = stall->reason();
    mDetectedMEGASide = stall->detectedCloudSide();

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

bool StalledIssue::isSolved() const
{
    return mIsSolved;
}

void StalledIssue::setIsSolved(bool isCloud)
{
    mIsSolved = true;

    if(!isCloud && consultLocalData())
    {
        getLocalData()->setIsSolved(true);
    }
    else if(isCloud && consultCloudData())
    {
        getCloudData()->setIsSolved(true);
    }
}

const StalledIssueDataPtr StalledIssue::consultLocalData() const
{
    return mLocalData;
}

const StalledIssueDataPtr StalledIssue::consultCloudData() const
{
    return mCloudData;
}

const QExplicitlySharedDataPointer<StalledIssueData> &StalledIssue::getLocalData() const
{
    return mLocalData;
}

const QExplicitlySharedDataPointer<StalledIssueData> &StalledIssue::getCloudData() const
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

bool StalledIssue::operator==(const StalledIssue &data)
{
    bool equal(true);

    equal &= (mLocalData == data.getLocalData());
    equal &= (mCloudData == data.getCloudData());

    return equal;
}

void StalledIssue::updateIssue(const mega::MegaSyncStall *stallIssue)
{
    mLocalData.reset();
    mCloudData.reset();

    mIsSolved = false;

    fillIssue(stallIssue);
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
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        {
            return StalledIssueFilterCriterion::NAME_CONFLICTS;
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

//Name conflict Stalled Issue
NameConflictedStalledIssue::NameConflictedStalledIssue(const NameConflictedStalledIssue &tdr)
    : StalledIssue(tdr)
{

}

NameConflictedStalledIssue::NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue()
{
    mReason = stallIssue->reason();
    fillIssue(stallIssue);
}

void NameConflictedStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    auto localConflictNames = stall->pathCount(false);

    if(localConflictNames > 0)
    {
        initLocalIssue();

        for(unsigned int index = 0; index < localConflictNames; ++index)
        {
            QFileInfo localPath(QString::fromUtf8(stall->path(false,index)));

            ConflictedNameInfo info(localPath.fileName());
            mLocalConflictedNames.append(info);

            if(consultLocalData()->mPath.isEmpty())
            {
                getLocalData()->mPath.path = localPath.filePath();
            }
        }
    }

    auto cloudConflictNames = stall->pathCount(true);

    if(cloudConflictNames > 0)
    {
        initCloudIssue();

        for(unsigned int index = 0; index < cloudConflictNames; ++index)
        {
            QFileInfo cloudPath(QString::fromUtf8(stall->path(true,index)));

            ConflictedNameInfo info(cloudPath.fileName());
            mCloudConflictedNames.append(info);

            if(consultCloudData()->mPath.isEmpty())
            {
                getCloudData()->mPath.path = cloudPath.filePath();
            }
        }
    }
}

QStringList NameConflictedStalledIssue::convertConflictedNames(bool cloud, const mega::MegaSyncStall *stall)
{
    QStringList names;

    auto conflictNamesCount = stall->pathCount(cloud);

    for(unsigned int index = 0; index < conflictNamesCount; ++index)
    {
        QFileInfo cloudPath(QString::fromUtf8(stall->path(cloud,index)));
        names.append(cloudPath.fileName());
    }

    return names;
}

void NameConflictedStalledIssue::updateIssue(const mega::MegaSyncStall *stallIssue)
{
   mCloudConflictedNames.clear();
   mLocalConflictedNames.clear();

   mLocalData.reset();
   mCloudData.reset();

   fillIssue(stallIssue);
}

NameConflictedStalledIssue::NameConflictData NameConflictedStalledIssue::getNameConflictLocalData() const
{
    NameConflictData data;
    data.conflictedNames = mLocalConflictedNames;
    data.data = consultLocalData();
    data.isCloud = false;

    return data;
}

NameConflictedStalledIssue::NameConflictData NameConflictedStalledIssue::getNameConflictCloudData() const
{
    NameConflictData data;
    data.conflictedNames = mCloudConflictedNames;
    data.data = consultCloudData();
    data.isCloud = true;

    return data;
}

bool NameConflictedStalledIssue::solveLocalConflictedName(const QString &name, ConflictedNameInfo::SolvedType type)
{
    for (auto it = mLocalConflictedNames.begin(); it != mLocalConflictedNames.end(); ++it)
    {
        if((*it).conflictedName == name)
        {
            (*it).solved = type;
            return true;
        }
    }

    return false;
}

bool NameConflictedStalledIssue::solveCloudConflictedName(const QString &name, NameConflictedStalledIssue::ConflictedNameInfo::SolvedType type)
{
    for (auto it = mCloudConflictedNames.begin(); it != mCloudConflictedNames.end(); ++it)
    {
        if((*it).conflictedName == name)
        {
            (*it).solved = type;
            return true;
        }
    }

    return false;
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRename(const QString &name, const QString &renameTo)
{
    for (auto it = mLocalConflictedNames.begin(); it != mLocalConflictedNames.end(); ++it)
    {
        if((*it).conflictedName == name)
        {
            (*it).solved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;;
            (*it).renameTo = renameTo;
            return true;
        }
    }

    return false;
}

bool NameConflictedStalledIssue::solveCloudConflictedNameByRename(const QString &name, const QString &renameTo)
{
    for (auto it = mCloudConflictedNames.begin(); it != mCloudConflictedNames.end(); ++it)
    {
        if((*it).conflictedName == name)
        {
            (*it).solved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
            (*it).renameTo = renameTo;
            return true;
        }
    }

    return false;
}
