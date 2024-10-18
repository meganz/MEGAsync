#include "DownloadFileIssue.h"

#include <MegaApplication.h>
#include <StalledIssuesUtilities.h>

StalledIssueSPtr DownloadFileIssueFactory::createAndFillIssue(const mega::MegaSyncStall* stall)
{
    StalledIssueSPtr issue;

    if (stall->pathProblem(true, 0) == mega::MegaSyncStall::DownloadToTmpDestinationFailed)
    {
        issue = std::make_shared<UnknownDownloadIssue>(stall);
    }
    else if (stall->pathProblem(true, 0) == mega::MegaSyncStall::CloudNodeInvalidFingerprint)
    {
        issue = std::make_shared<InvalidFingerprintDownloadIssue>(stall);
    }
    else if (stall->pathProblem(true, 0) == mega::MegaSyncStall::CloudNodeIsBlocked)
    {
        issue = std::make_shared<DownloadIssue>(DownloadIssue::IssueType::NODEBLOCKED, stall);
    }
    else
    {
        issue = std::make_shared<DownloadIssue>(std::nullopt, stall);
    }

    return issue;
}

////////////////////////////////
/// \brief DownloadIssue::DownloadIssue
/// \param stall
///
std::optional<DownloadIssue::IssueType> DownloadIssue::getType() const
{
    return mType;
}

DownloadIssue::DownloadIssue(const mega::MegaSyncStall* stall):
    StalledIssue(stall)
{}

DownloadIssue::DownloadIssue(std::optional<IssueType> type, const mega::MegaSyncStall* stall):
    StalledIssue(stall),
    mType(type)
{}

////////////////////////////////
/// \brief UnknownDownloadIssue::UnknownDownloadIssue
/// \param stall
///

QList<mega::MegaHandle> UnknownDownloadIssue::mIssuesToRetry = QList<mega::MegaHandle>();

UnknownDownloadIssue::UnknownDownloadIssue(const mega::MegaSyncStall* stall):
    DownloadIssue(stall)
{
    mType = IssueType::UNKNOWN;

    connect(MegaSyncApp->getTransfersModel(),
            &TransfersModel::retryableSyncTransferRetried,
            this,
            &UnknownDownloadIssue::onRetryableSyncTransferRetried);

    connect(MegaSyncApp->getTransfersModel(),
            &TransfersModel::retriedSyncTransferFinished,
            this,
            &UnknownDownloadIssue::onRetriedSyncTransferFinished);
}

void UnknownDownloadIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    DownloadIssue::fillIssue(stall);

    std::shared_ptr<DownloadTransferInfo> info(new DownloadTransferInfo());
    info->state = TransferData::ACTIVE_STATES_MASK | TransferData::TRANSFER_COMPLETED;
    if (isBeingSolvedByDownload(info))
    {
        setIsSolved(SolveType::SOLVED);
    }
}

bool UnknownDownloadIssue::canBeRetried() const
{
    std::shared_ptr<DownloadTransferInfo> info(new DownloadTransferInfo());
    info->state = TransferData::TRANSFER_FAILED;
    return isBeingSolvedByDownload(info);
}

bool UnknownDownloadIssue::checkForExternalChanges()
{
    return !canBeRetried();
}

void UnknownDownloadIssue::solveIssues()
{
    MegaSyncApp->getTransfersModel()->retrySyncFailedTransfers(mIssuesToRetry);
    mIssuesToRetry.clear();
}

void UnknownDownloadIssue::addIssueToSolve(const StalledIssueVariant& issueToFix)
{
    if (issueToFix.isValid() && issueToFix.consultData()->consultCloudData())
    {
        mIssuesToRetry.append(issueToFix.consultData()->consultCloudData()->getPathHandle());
    }
}

void UnknownDownloadIssue::onRetryableSyncTransferRetried(mega::MegaHandle handle)
{
    if (handle == consultCloudData()->getPathHandle())
    {
        setIsSolved(SolveType::BEING_SOLVED);
    }
}

void UnknownDownloadIssue::onRetriedSyncTransferFinished(mega::MegaHandle handle,
                                                         TransferData::TransferState state)
{
    if (handle == consultCloudData()->getPathHandle())
    {
        if (state == TransferData::TransferState::TRANSFER_FAILED)
        {
            setIsSolved(SolveType::FAILED);
        }
        else
        {
            setIsSolved(SolveType::SOLVED);
        }
    }

    emit asyncIssueSolvingFinished(this);
}

////////////////////////////////
/// \brief InvalidFingerprintDownloadIssue::InvalidFingerprintDownloadIssue
/// \param stall
///
///

QList<StalledIssueVariant> InvalidFingerprintDownloadIssue::mFingerprintIssuesToFix =
    QList<StalledIssueVariant>();

InvalidFingerprintDownloadIssue::InvalidFingerprintDownloadIssue(const mega::MegaSyncStall* stall):
    DownloadIssue(stall)
{
    mType = IssueType::FINGERPRINT;
}

void InvalidFingerprintDownloadIssue::solveIssues()
{
    QDir dir(Preferences::instance()->getTempTransfersPath());
    if (dir.exists() || dir.mkpath(QString::fromUtf8(".")))
    {
        auto tempPath(Preferences::instance()->getTempTransfersPath());

        QMap<QString, std::shared_ptr<QQueue<WrappedNode*>>> nodesToDownloadByPath;
        auto appendNodeToQueue = [&](const QString& targetPath, mega::MegaNode* node)
        {
            if (!nodesToDownloadByPath.contains(targetPath))
            {
                auto queue(std::make_shared<QQueue<WrappedNode*>>());
                queue->append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
                nodesToDownloadByPath.insert(targetPath, queue);
            }
            else
            {
                auto queue = nodesToDownloadByPath.value(targetPath);
                if (queue)
                {
                    queue->append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
                }
            }
        };

        foreach(auto issue, mFingerprintIssuesToFix)
        {
            std::shared_ptr<DownloadTransferInfo> info(new DownloadTransferInfo());
            info->state = TransferData::ACTIVE_STATES_MASK;
            if (!issue.consultData()->isBeingSolvedByDownload(info))
            {
                mega::MegaNode* node(MegaSyncApp->getMegaApi()->getNodeByHandle(info->nodeHandle));

                auto localPath = issue.consultData()->consultLocalData() ?
                                     issue.consultData()->consultLocalData()->getNativeFilePath() :
                                     QString();
                if (!localPath.isEmpty())
                {
                    QFile localPathFile(localPath);
                    if (!localPathFile.exists())
                    {
                        QFileInfo localFilePathInfo(localPath);
                        QString localFolderPath(localFilePathInfo.absolutePath());
                        QDir createTarget;
                        createTarget.mkdir(localFolderPath);
                        QDir localFolderPathDir(localFolderPath);
                        if (localFolderPathDir.exists())
                        {
                            appendNodeToQueue(localFolderPath, node);
                            continue;
                        }
                    }
                }

                if (node)
                {
                    appendNodeToQueue(tempPath, node);
                }
            }
        }

        foreach(auto targetFolder, nodesToDownloadByPath.keys())
        {
            std::shared_ptr<QQueue<WrappedNode*>> nodesToDownload(
                nodesToDownloadByPath.value(targetFolder));
            StalledIssuesUtilities::getMegaDownloader()->processTempDownloadQueue(
                nodesToDownload.get(),
                targetFolder);
            qDeleteAll(*nodesToDownload.get());
        }

        mFingerprintIssuesToFix.clear();
    }
}

void InvalidFingerprintDownloadIssue::addIssueToSolve(const StalledIssueVariant& issueToFix)
{
    mFingerprintIssuesToFix.append(issueToFix);
}
