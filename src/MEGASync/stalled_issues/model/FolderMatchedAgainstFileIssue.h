#ifndef FOLDERMATCHEDAGAINSTFILEISSUE_H
#define FOLDERMATCHEDAGAINSTFILEISSUE_H

#include "StalledIssue.h"
#include "StalledIssuesUtilities.h"

class FolderMatchedAgainstFileIssue : public StalledIssue
{
public:
    FolderMatchedAgainstFileIssue(const mega::MegaSyncStall *stallIssue);
    ~FolderMatchedAgainstFileIssue() = default;

    bool solveIssue();

    const StalledIssuesUtilities::KeepBothSidesState& getResult() const;

    bool UIShowFileAttributes() const override;

private:
    StalledIssuesUtilities::KeepBothSidesState mResult;

};

#endif // FOLDERMATCHEDAGAINSTFILEISSUE_H
