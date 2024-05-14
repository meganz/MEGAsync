#ifndef MOVEORRENAMECANNOTOCCURISSUE_H
#define MOVEORRENAMECANNOTOCCURISSUE_H

#include <StalledIssue.h>
#include <StalledIssuesFactory.h>

class SyncController;
class SyncSettings;

enum class MoveOrRenameIssueChosenSide
{
    NONE,
    LOCAL,
    REMOTE,
};

class MoveOrRenameCannotOccurIssue : public StalledIssue
{
    Q_OBJECT

public:

    MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall *stall);

    void fillIssue(const mega::MegaSyncStall*) override;
    void fillCloudSide(const mega::MegaSyncStall* stall);
    void fillLocalSide(const mega::MegaSyncStall* stall);

    bool isAutoSolvable() const override;
    bool refreshListAfterSolving() const override;
    void solveIssue(MoveOrRenameIssueChosenSide side);
    bool autoSolveIssue() override;

    bool checkForExternalChanges() override;

    MoveOrRenameIssueChosenSide getChosenSide() const;
    MoveOrRenameIssueChosenSide getSyncIdChosenSide() const;

    void increaseCombinedNumberOfIssues();
    int combinedNumberOfIssues() const;

    static bool findIssue(const std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue);
    static void solvingIssueInSeveralStepsFinished();

signals:
    void issueBeingSolved();

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    bool mSolvingStarted;
    std::shared_ptr<SyncController> mSyncController;
    static QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> mChosenSideBySyncId;
    MoveOrRenameIssueChosenSide mChosenSide;
    int mCombinedNumberOfIssues;
};

class MoveOrRenameCannotOccurFactory : public StalledIssuesFactory
{
public:
    MoveOrRenameCannotOccurFactory(){}
    ~MoveOrRenameCannotOccurFactory() = default;

    std::shared_ptr<StalledIssue> createIssue(const mega::MegaSyncStall* stall) override;
    void clear() override { mIssueBySyncId.clear(); }

private:
    QHash<mega::MegaHandle, std::shared_ptr<MoveOrRenameCannotOccurIssue>> mIssueBySyncId;
};

class MoveOrRenameMultiStepIssueSolver : public MultiStepIssueSolver<MoveOrRenameCannotOccurIssue>
{
    Q_OBJECT

public:
    MoveOrRenameMultiStepIssueSolver(
     std::shared_ptr<MoveOrRenameCannotOccurIssue> issue)
        : MultiStepIssueSolver<MoveOrRenameCannotOccurIssue>(issue)
    {
        mSyncId = issue->syncIds().first();
    }

    bool checkIssue(std::shared_ptr<StalledIssue> d) override
    {
        return d->syncIds().contains(mSyncId);
    }

private:
    mega::MegaHandle mSyncId;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
