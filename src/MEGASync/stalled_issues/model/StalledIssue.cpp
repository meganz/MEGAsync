#include "StalledIssue.h"

#include "MegaApplication.h"

StalledIssueData::StalledIssueData(std::unique_ptr<mega::MegaSyncStall> originalstall)
    : original(std::move(originalstall))
    , mIsCloud(false)
    , mIsSolved(false)
{
    qRegisterMetaType<StalledIssueDataPtr>("StalledIssueDataPtr");
    qRegisterMetaType<StalledIssuesDataList>("StalledIssuesDataList");

    qRegisterMetaType<StalledIssue>("StalledIssue");

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
    QString path;

    if(isCloud())
    {
        path = mPath.path;
    }
    else
    {
        QFileInfo filePath(mPath.path);
        path = QDir::toNativeSeparators(filePath.filePath());
    }

    return path;
}

QString StalledIssueData::getNativeMoveFilePath() const
{
    QString path;

    if(isCloud())
    {
        path = mMovePath.path;
    }
    else
    {
        QFileInfo filePath(mMovePath.path);
        path =  QDir::toNativeSeparators(filePath.filePath());
    }

    return path;
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
    QString fileName;

    if(filePath.isFile())
    {
        fileName = filePath.fileName();
    }
    else
    {
        if(isCloud())
        {
            auto splittedIndexPath = mPath.path.split(QString::fromUtf8("/"));
            fileName = splittedIndexPath.last();
        }
        else
        {
            auto splittedIndexPath = mPath.path.split(QString::fromUtf8("\\"));
            fileName = splittedIndexPath.last();
        }
    }

    checkTrailingSpaces(fileName);

    return fileName;
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

void StalledIssueData::checkTrailingSpaces(QString &name) const
{
    auto trimmedPath = name.trimmed();
    if(trimmedPath != name)
    {
        name.prepend(QString::fromUtf8("\""));
        name.append(QString::fromUtf8("\""));
    }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief StalledIssue::StalledIssue
/// \param stallIssue
///
StalledIssue::StalledIssue(const mega::MegaSyncStall *stallIssue)
{
    fillIssue(stallIssue);
    originalStall.reset(stallIssue->copy());
}

bool StalledIssue::initLocalIssue(const mega::MegaSyncStall *stallIssue)
{
    if(!mLocalData)
    {
        mLocalData = QExplicitlySharedDataPointer<StalledIssueData>(new StalledIssueData(std::unique_ptr<mega::MegaSyncStall>(stallIssue->copy())));
        return true;
    }

    return false;
}

bool StalledIssue::initCloudIssue(const mega::MegaSyncStall *stallIssue)
{
    if(!mCloudData)
    {
        mCloudData = QExplicitlySharedDataPointer<StalledIssueData>(new StalledIssueData(std::unique_ptr<mega::MegaSyncStall>(stallIssue->copy())));
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
        initLocalIssue(stall);
        getLocalData()->mPath.path = localSourcePath;
        getLocalData()->mPath.mPathProblem = localSourcePathProblem;

        if(stall->couldSuggestIgnoreThisPath(false, 0))
        {
            mIgnoredPaths.append(getLocalData()->getNativeFilePath());
        }

        setIsFile(localSourcePath, true);
    }

    if(localTargetPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !localTargetPath.isEmpty())
    {
        initLocalIssue(stall);
        getLocalData()->mMovePath.path = localTargetPath;
        getLocalData()->mMovePath.mPathProblem = localTargetPathProblem;

        if(stall->couldSuggestIgnoreThisPath(false, 1))
        {
            mIgnoredPaths.append(getLocalData()->getNativeMoveFilePath());
        }

        setIsFile(localTargetPath, true);
    }

    auto cloudSourcePathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(true,0));
    auto cloudTargetPathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(true,1));

    auto cloudSourcePath = QString::fromUtf8(stall->path(true,0));
    auto cloudTargetPath = QString::fromUtf8(stall->path(true,1));

    if(cloudSourcePathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !cloudSourcePath.isEmpty())
    {
        initCloudIssue(stall);
        getCloudData()->mPath.path = cloudSourcePath;
        getCloudData()->mPath.mPathProblem = cloudSourcePathProblem;

        if(stall->couldSuggestIgnoreThisPath(true, 0))
        {
            mIgnoredPaths.append(cloudSourcePath);
        }

        setIsFile(cloudSourcePath, false);
    }

    if(cloudTargetPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !cloudTargetPath.isEmpty())
    {
        initCloudIssue(stall);
        getCloudData()->mMovePath.path = cloudTargetPath;
        getCloudData()->mMovePath.mPathProblem = cloudTargetPathProblem;

        if(stall->couldSuggestIgnoreThisPath(true, 1))
        {
            mIgnoredPaths.append(cloudTargetPath);
        }

        setIsFile(cloudTargetPath, false);
    }
}

const std::shared_ptr<mega::MegaSyncStall> &StalledIssue::getOriginalStall() const
{
    return originalStall;
}

uint8_t StalledIssue::hasFiles() const
{
    return mFiles;
}

uint8_t StalledIssue::hasFolders() const
{
    return mFolders;
}

const QSize &StalledIssue::getDelegateSize(SizeType type) const
{
    switch(type)
    {
        case SizeType::Header:
            return mHeaderDelegateSize;
        case SizeType::Body:
            return mBodyDelegateSize;
    }
}

void StalledIssue::setDelegateSize(const QSize &newDelegateSize, SizeType type)
{
    switch(type)
    {
        case SizeType::Header:
            mHeaderDelegateSize = newDelegateSize;
            break;
        case SizeType::Body:
            mBodyDelegateSize = newDelegateSize;
            break;
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

bool StalledIssue::canBeIgnored() const
{
    return !mIgnoredPaths.isEmpty();
}

QStringList StalledIssue::getIgnoredFiles() const
{
    return mIgnoredPaths;
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

void StalledIssue::setIsFile(const QString &path, bool isLocal)
{
    if(isLocal)
    {
        QFileInfo fileInfo(path);
        fileInfo.isFile() ? mFiles++ : mFolders++;
    }
    else
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(path.toStdString().c_str()));
        if(node)
        {
            node->isFile()  ? mFiles++ : mFolders++;
        }
    }
}

mega::MegaSyncStall::SyncStallReason StalledIssue::getReason() const
{
    return mReason;
}

QString StalledIssue::getFileName(bool preferCloud) const
{
    QString fileName;

    if (preferCloud)
    {
        if (mCloudData)
        {
            fileName = mCloudData->getFileName();
        }
    }
    else
    {
        if (mLocalData)
        {
            fileName = mLocalData->getFileName();
        }
    }

    if(mLocalData)
    {
        fileName = mLocalData->getFileName();
    }
    else if(mCloudData)
    {
        fileName = mCloudData->getFileName();
    }

    return fileName;
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
