#include "MoveOrRenameCannotOccurIssue.h"

#include <syncs/control/SyncInfo.h>
#include <syncs/control/SyncController.h>
#include <syncs/control/SyncSettings.h>

#include <MegaApplication.h>
#include <Utilities.h>

#include <QDir>

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall)
    : StalledIssue(stall),
    QObject(nullptr),
    mSolvingStarted(false),
    mSyncController(new SyncController())
{
    qDebug() << connect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
            this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);
}

void MoveOrRenameCannotOccurIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssue::fillIssue(stall);

    auto localMovePath = getLocalData()->getMovePath();
    if(localMovePath.mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem)
    {
        QFileInfo info(localMovePath.path);
        mPathToCreate.path = info.absolutePath();
        mPathToCreate.isCloud = false;
    }
    else
    {
        auto cloudMovePath = getCloudData()->getMovePath();
        if(cloudMovePath.mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem)
        {
            QFileInfo info(cloudMovePath.path);
            mPathToCreate.path = info.absolutePath();
            mPathToCreate.isCloud = true;
        }
    }
}

bool MoveOrRenameCannotOccurIssue::isSolvable() const
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
        if(mPathToCreate.isCloud)
        {
            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(mPathToCreate.path.toUtf8()));
            if(node)
            {
                setIsSolved(true);
            }
        }
        else
        {
            QFileInfo fileInfo(mPathToCreate.path);
            if(fileInfo.exists())
            {
                setIsSolved(true);
            }
        }
    }

    return isPotentiallySolved();
}

const QString &MoveOrRenameCannotOccurIssue::pathToCreate() const
{
    return mPathToCreate.path;
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

        if(mPathToCreate.isCloud)
        {
            CreateDirectory dirCreator;
            created = dirCreator.mkDir(QString(), mPathToCreate.path);
        }
        else
        {
            created = QDir().mkdir(mPathToCreate.path);
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
