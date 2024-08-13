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
