#include "MoveOrRenameCannotOccurIssue.h"

#include <syncs/control/SyncInfo.h>
#include <syncs/control/SyncController.h>
#include <syncs/control/SyncSettings.h>

#include <MegaApplication.h>
#include <Utilities.h>

#include <QDir>

StalledIssueVariant MoveOrRenameCannotOccurFactory::createIssue(
    const mega::MegaSyncStall* stall)
{
    auto newIssue = std::make_shared<MoveOrRenameCannotOccurIssue>(stall);
    auto previousIssue = mIssueBySyncId.value(newIssue->syncIds().first());
    if(previousIssue.isValid())
    {
        auto moveOrRenameIssue = previousIssue.convert<MoveOrRenameCannotOccurIssue>();
        if(moveOrRenameIssue)
        {
            stall->detectedCloudSide() ? moveOrRenameIssue->fillCloudSide(stall)
                                       : moveOrRenameIssue->fillLocalSide(stall);
        }

        return StalledIssueVariant();
    }
    else
    {
        StalledIssueVariant variant(newIssue, stall);
        stall->detectedCloudSide() ? newIssue->fillCloudSide(stall) : newIssue->fillLocalSide(stall);
        mIssueBySyncId.insert(newIssue->syncIds().first(), variant);
        return variant;
    }
}

//////////////////////////////////////

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall)
    : QObject(nullptr)
    , StalledIssue(stall)
    , mSolvingStarted(false)
    , mSyncController(new SyncController())
    , mSideChosen(SideChosen::NONE)
{
    connect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
            this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

    fillBasicInfo(stall);
}

//We don´t fill the issue as usual
void MoveOrRenameCannotOccurIssue::fillIssue(const mega::MegaSyncStall*)
{}

bool MoveOrRenameCannotOccurIssue::isSolvable() const
{
    if (consultLocalData() && consultCloudData())
    {
        std::unique_ptr<mega::MegaNode> previousNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
        std::unique_ptr<mega::MegaNode> currentNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
        if (previousNode && currentNode)
        {
            return !isSolved();
        }
        else
        {
            QFileInfo previousPath(consultLocalData()->getPath().path);
            QFileInfo currentPath(consultLocalData()->getMovePath().path);
            if (previousPath.exists() && currentPath.exists())
            {
                return !isSolved();
            }
        }
    }

    return true;
}

bool MoveOrRenameCannotOccurIssue::refreshListAfterSolving() const
{
    return true;
}

void MoveOrRenameCannotOccurIssue::solveIssue(SideChosen side)
{
    mSideChosen = /*side*/SideChosen::LOCAL;

    if(!syncIds().isEmpty())
    {
        auto syncId(syncIds().first());
        auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(syncId);
        if(syncSettings)
        {
            mSolvingStarted = true;
            mSyncController->setSyncToPause(syncSettings);
        }
    }
}

bool MoveOrRenameCannotOccurIssue::checkForExternalChanges()
{
    if(!isSolved())
    {
        std::unique_ptr<mega::MegaNode> previousNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
        std::unique_ptr<mega::MegaNode> currentNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
        if (!previousNode || !currentNode)
        {
            setIsSolved(true);
        }
        else
        {
            QFileInfo previousPath(consultLocalData()->getPath().path);
            QFileInfo currentPath(consultLocalData()->getMovePath().path);
            if (!previousPath.absoluteDir().exists() || !currentPath.exists())
            {
                setIsSolved(true);
            }
        }
    }

    return isPotentiallySolved();
}

void MoveOrRenameCannotOccurIssue::onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings)
{
    if(!mSolvingStarted)
    {
        return;
    }
    const auto syncState = syncSettings->getRunState();
    if(syncState == mega::MegaSync::RUNSTATE_PAUSED || syncState == mega::MegaSync::RUNSTATE_SUSPENDED)
    {
        bool created(false);

        //We perform the undo in the opposite side
        if(mSideChosen == SideChosen::REMOTE)
        {
            QFileInfo previousPath(consultLocalData()->getFilePath());
            QFileInfo previousDirectory(previousPath.absolutePath());

            created = previousDirectory.exists();

            if(!created)
            {
                created = QDir().mkpath(previousDirectory.absoluteFilePath());
            }

            if(created)
            {
                QFile file(consultLocalData()->getNativeMoveFilePath());
                created = file.rename(previousPath.absoluteFilePath());
            }
        }
        else
        {
            std::unique_ptr<mega::MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
            if(nodeToMove)
            {
                std::unique_ptr<mega::MegaNode> newParent(MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
                if(!newParent)
                {
                    PathCreator dirCreator;
                    dirCreator.mkDir(QString(), consultCloudData()->getNativeFilePath());
                }
                MegaSyncApp->getMegaApi()->moveNode(nodeToMove.get(), newParent.get());
                created = true;
            }
        }

        if(created)
        {
            disconnect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
                this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);
            setIsSolved(false);
        }

        emit issueSolved(created);

        mSyncController->setSyncToRun(syncSettings);
    }

    mSolvingStarted = false;
}

void MoveOrRenameCannotOccurIssue::fillCloudSide(const mega::MegaSyncStall* stall)
{
    auto cloudSourcePath = QString::fromUtf8(stall->path(true, 0));
    auto cloudTargetPath = QString::fromUtf8(stall->path(true, 1));

    if(!cloudSourcePath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mPath.path = cloudSourcePath;
        QFileInfo previousPath(cloudSourcePath);
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(previousPath.absolutePath().toStdString().c_str()));
        if(node)
        {
            getCloudData()->mPathHandle = node->getHandle();
        }

        setIsFile(cloudSourcePath, false);
    }

    if(!cloudTargetPath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mMovePath.path = cloudTargetPath;

        QFileInfo targetPath(cloudTargetPath);
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(targetPath.absoluteFilePath().toStdString().c_str()));
        if(node)
        {
            getCloudData()->mMovePathHandle = node->getHandle();
        }

        setIsFile(cloudTargetPath, false);
    }
}

void MoveOrRenameCannotOccurIssue::fillLocalSide(const mega::MegaSyncStall* stall)
{
    auto localSourcePath = QString::fromUtf8(stall->path(false, 0));
    auto localTargetPath = QString::fromUtf8(stall->path(false, 1));

    if(!localSourcePath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mPath.path = localSourcePath;

        setIsFile(localSourcePath, true);
    }

    if(!localTargetPath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mMovePath.path = localTargetPath;

        setIsFile(localTargetPath, true);
    }
}
