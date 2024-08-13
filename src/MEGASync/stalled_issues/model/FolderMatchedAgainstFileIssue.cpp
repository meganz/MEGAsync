#include "FolderMatchedAgainstFileIssue.h"
#include "StalledIssuesUtilities.h"

FolderMatchedAgainstFileIssue::FolderMatchedAgainstFileIssue(const mega::MegaSyncStall* stallIssue):
    StalledIssue(stallIssue)
{}

bool FolderMatchedAgainstFileIssue::solveIssue() {

    mResult = StalledIssuesUtilities::KeepBothSides(getCloudData()->getNode(), getLocalData()->getNativeFilePath());
    if(mResult.error != nullptr)
    {
        setIsSolved(SolveType::FAILED);
        return false;
    }
    else
    {
        //Stop pointing to the old local path as it will be pointing to the new REMOTE FOLDER
        if(mResult.sideRenamed == StalledIssuesUtilities::KeepBothSidesState::LOCAL)
        {
            QFileInfo fileInfo;
            fileInfo.setFile(getLocalData()->getNativePath(), mResult.newName);

            getLocalData()->mPath.path = fileInfo.filePath();
        }
    }

    return true;
}

const StalledIssuesUtilities::KeepBothSidesState& FolderMatchedAgainstFileIssue::getResult() const
{
    return mResult;
}

bool FolderMatchedAgainstFileIssue::UIShowFileAttributes() const
{
    return true;
}
