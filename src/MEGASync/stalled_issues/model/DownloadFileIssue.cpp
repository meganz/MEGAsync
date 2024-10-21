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
}

void UnknownDownloadIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    DownloadIssue::fillIssue(stall);

    mTrack = MegaSyncApp->getTransfersModel()->getTrackToTransfer(
        QString::number(getOriginalStall()->getHash()));

    // If the track exists, it is because the transfer is still being processes (active)
    if (mTrack)
    {
        connectTrack();
        setIsSolved(SolveType::BEING_SOLVED);
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

void UnknownDownloadIssue::addIssueToSolveQueue()
{
    if (isValid() && consultCloudData())
    {
        auto pathHandle(consultCloudData()->getPathHandle());

        if (!mTrack)
        {
            mTrack = MegaSyncApp->getTransfersModel()->addTrackToTransfer(
                QString::number(getOriginalStall()->getHash()),
                TransferData::TRANSFER_DOWNLOAD);
            connectTrack();
        }

        mTrack->addTransferToTrack(pathHandle);

        mIssuesToRetry.append(pathHandle);
    }
}

void UnknownDownloadIssue::onTrackedTransferStarted(TransferItem transfer)
{
    if (consultCloudData()->getPathHandle() == transfer.getTransferData()->mNodeHandle)
    {
        setIsSolved(SolveType::BEING_SOLVED);
    }
}

void UnknownDownloadIssue::onTrackedTransferFinished(TransferItem transfer)
{
    auto data(transfer.getTransferData());
    if (consultCloudData()->getPathHandle() == data->mNodeHandle)
    {
        performFinishAsyncIssueSolving(data->getState() ==
                                       TransferData::TransferState::TRANSFER_FAILED);
    }
}

void UnknownDownloadIssue::connectTrack()
{
    connect(mTrack.get(),
            &TransferTrack::transferStarted,
            this,
            &UnknownDownloadIssue::onTrackedTransferStarted);

    connect(mTrack.get(),
            &TransferTrack::transferFinished,
            this,
            &UnknownDownloadIssue::onTrackedTransferFinished);
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
