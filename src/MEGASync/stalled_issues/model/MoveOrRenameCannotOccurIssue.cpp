#include "MoveOrRenameCannotOccurIssue.h"

#include <syncs/control/SyncInfo.h>
#include <syncs/control/SyncController.h>
#include <syncs/control/SyncSettings.h>

#include <MegaApplication.h>
#include <Utilities.h>

#include <QDir>

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall)
    : QObject(nullptr),
    StalledIssue(stall),
    mSolvingStarted(false),
    mSyncController(new SyncController()),
    mIsSolvable(false)
{
    connect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
            this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);
}

void MoveOrRenameCannotOccurIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssue::fillIssue(stall);

    QFileInfo currentPath(getLocalData()->getMoveFilePath());

    //First check if we can undo the local movement or rename
    if(currentPath.exists())
    {
        mPathToSolve.isFile = currentPath.isFile();

        mPathToSolve.previousPath = getLocalData()->getFilePath();
        mPathToSolve.currentPath = getLocalData()->getMoveFilePath();

        mPathToSolve.isCloud = false;
    }
    else
    {
        QFileInfo previousPath(getLocalData()->getFilePath());
        currentPath = QFile(getCloudData()->getMoveFilePath());

        std::unique_ptr<mega::MegaNode> currentNode(MegaSyncApp->getMegaApi()->getNodeByPath(currentPath.absoluteFilePath().toUtf8()));
        if(currentNode)
        {
            mPathToSolve.isFile = currentNode->isFile();

            previousPath = QFile(getCloudData()->getFilePath());
            std::unique_ptr<mega::MegaNode> previousParentNode(MegaSyncApp->getMegaApi()->getNodeByPath(previousPath.absolutePath().toUtf8()));
            if(previousParentNode)
            {
                mPathToSolve.previousHandle = previousParentNode->getHandle();
            }
            mPathToSolve.currentHandle = currentNode->getHandle();

            mPathToSolve.previousPath = getCloudData()->getFilePath();
            mPathToSolve.currentPath = getCloudData()->getMoveFilePath();

            mPathToSolve.isCloud = true;
        }
    }
}

bool MoveOrRenameCannotOccurIssue::isSolvable() const
{
    if(mPathToSolve.isCloud &&
        mPathToSolve.currentHandle != mega::INVALID_HANDLE)
    {
        return !isSolved();
    }
    else if(!mPathToSolve.isCloud &&
            !mPathToSolve.currentPath.isEmpty())
    {
        return !isSolved();
    }

    return false;
}

bool MoveOrRenameCannotOccurIssue::refreshListAfterSolving() const
{
    return true;
}

void MoveOrRenameCannotOccurIssue::solveIssue()
{
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
        if(mPathToSolve.isCloud)
        {
            QFileInfo currentInfo(getLocalData()->getMovePath().path);
            std::unique_ptr<mega::MegaNode> currentNode(MegaSyncApp->getMegaApi()->getNodeByPath(mPathToSolve.currentPath.toUtf8()));
            if(currentInfo.exists() || !currentNode)
            {
                setIsSolved(true);
            }
        }
        else
        {
            QFileInfo currentInfo(mPathToSolve.currentPath);
            std::unique_ptr<mega::MegaNode> previousNode(MegaSyncApp->getMegaApi()->getNodeByPath(getCloudData()->getMovePath().path.toUtf8()));
            if(previousNode || !currentInfo.exists())
            {
                setIsSolved(true);
            }
        }
    }

    return isPotentiallySolved();
}

const QString& MoveOrRenameCannotOccurIssue::currentPath() const
{
    return mPathToSolve.currentPath;
}

QString MoveOrRenameCannotOccurIssue::previousPath() const
{
    QFileInfo previousPath(mPathToSolve.previousPath);
    return previousPath.absolutePath();
}

bool MoveOrRenameCannotOccurIssue::isFile() const
{
    return mPathToSolve.isFile;
}

void MoveOrRenameCannotOccurIssue::onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings)
{
    if(!mSolvingStarted)
    {
        return;
    }

    if(syncSettings->getRunState() == mega::MegaSync::RUNSTATE_PAUSED)
    {
        bool created(false);

        if(mPathToSolve.isCloud)
        {            
            std::unique_ptr<mega::MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(mPathToSolve.currentHandle));
            if(nodeToMove)
            {
                std::unique_ptr<mega::MegaNode> newParent(MegaSyncApp->getMegaApi()->getNodeByHandle(mPathToSolve.previousHandle));
                if(!newParent)
                {
                    CreateDirectory dirCreator;
                    dirCreator.mkDir(QString(), mPathToSolve.previousPath);
                }
                MegaSyncApp->getMegaApi()->moveNode(nodeToMove.get(), newParent.get());
                created = true;
            }
        }
        else
        {
            QFileInfo previousPath(mPathToSolve.previousPath);
            QFileInfo previousDirectory(previousPath.absolutePath());

            created = previousDirectory.exists();

            if(!created)
            {
                created = QDir().mkpath(previousDirectory.absoluteFilePath());
            }

            if(created)
            {
                QFile file(mPathToSolve.currentPath);
                created = file.rename(mPathToSolve.previousPath);
            }
        }

        if(created)
        {
            setIsSolved(false);
        }

        emit issueSolved(created);

        mSyncController->setSyncToRun(syncSettings);
    }

    mSolvingStarted = false;
}
