#include "NameConflictStalledIssue.h"

//Name conflict Stalled Issue
NameConflictedStalledIssue::NameConflictedStalledIssue(const NameConflictedStalledIssue &tdr)
    : StalledIssue(tdr)
{
    qRegisterMetaType<NameConflictedStalledIssue>("NameConflictedStalledIssue");
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
        initLocalIssue(stall);

        for(unsigned int index = 0; index < localConflictNames; ++index)
        {
            QFileInfo localPath(QString::fromUtf8(stall->path(false,index)));

            ConflictedNameInfo info(localPath.fileName());
            mLocalConflictedNames.append(info);

            if(consultLocalData()->mPath.isEmpty())
            {
                getLocalData()->mPath.path = localPath.filePath();
            }

            setIsFile(localPath.filePath(), true);
        }
    }

    auto cloudConflictNames = stall->pathCount(true);

    if(cloudConflictNames > 0)
    {
        initCloudIssue(stall);

        for(unsigned int index = 0; index < cloudConflictNames; ++index)
        {
            QFileInfo cloudPath(QString::fromUtf8(stall->path(true,index)));

            ConflictedNameInfo info(cloudPath.fileName());
            mCloudConflictedNames.append(info);

            if(consultCloudData()->mPath.isEmpty())
            {
                getCloudData()->mPath.path = cloudPath.filePath();
            }

            setIsFile(cloudPath.filePath(), false);
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

bool NameConflictedStalledIssue::solveLocalConflictedName(const QString &name, int conflictIndex, ConflictedNameInfo::SolvedType type)
{
    auto result(false);
    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];
        conflictName.solved = type;

        result = checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mCloudConflictedNames);
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveCloudConflictedName(const QString &name, int conflictIndex, NameConflictedStalledIssue::ConflictedNameInfo::SolvedType type)
{
    auto result(false);
    if(mCloudConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mCloudConflictedNames[conflictIndex];
        conflictName.solved = type;

        result = checkAndSolveConflictedNamesSolved(mCloudConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRename(const QString &name, int conflictIndex, const QString &renameTo)
{
    auto result(false);

    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];
        conflictName.solved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;;
        conflictName.renameTo = renameTo;

        result = checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mCloudConflictedNames);
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveCloudConflictedNameByRename(int conflictIndex, const QString &renameTo)
{
    auto result(false);

    if(mCloudConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mCloudConflictedNames[conflictIndex];
        conflictName.solved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;;
        conflictName.renameTo = renameTo;

        result = checkAndSolveConflictedNamesSolved(mCloudConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        }
    }

    return result;
}

bool NameConflictedStalledIssue::checkAndSolveConflictedNamesSolved(QList<ConflictedNameInfo>& conflicts)
{
    auto unsolvedItems(0);

    for (auto it = conflicts.begin(); it != conflicts.end(); ++it)
    {
        if(!(*it).isSolved())
        {
            unsolvedItems++;
        }
    }

    if(unsolvedItems < 2)
    {
        for (auto it = conflicts.begin(); it != conflicts.end(); ++it)
        {
            if(!(*it).isSolved())
            {
                (*it).solved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
                unsolvedItems--;
            }
        }
    }

    return unsolvedItems == 0;
}

