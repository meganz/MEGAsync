#ifndef MOVEORRENAMECANNOTOCCURISSUE_H
#define MOVEORRENAMECANNOTOCCURISSUE_H

#include <StalledIssue.h>
#include <StalledIssuesFactory.h>
#include <StalledIssuesUtilities.h>

#include <QTMegaRequestListener.h>

class SyncSettings;

enum class MoveOrRenameIssueChosenSide
{
    NONE = 0x0,
    LOCAL = 0x1,
    REMOTE = 0x2,
};

Q_DECLARE_FLAGS(ChoosableSides, MoveOrRenameIssueChosenSide)
Q_DECLARE_OPERATORS_FOR_FLAGS(ChoosableSides)

class MoveOrRenameCannotOccurIssue : public StalledIssue, public mega::MegaRequestListener
{
    Q_OBJECT

public:

    MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall *stall);
    ~MoveOrRenameCannotOccurIssue() override{}

    bool isValid() const override;

    void fillIssue(const mega::MegaSyncStall* stall) override;

    void setIsSolved(SolveType type) override;
    bool isAutoSolvable() const override;
    void solveIssue(MoveOrRenameIssueChosenSide side);
    StalledIssue::AutoSolveIssueResult autoSolveIssue() override;

    bool isKeepSideAvailable(MoveOrRenameIssueChosenSide side) const;

    bool checkForExternalChanges() override;

    MoveOrRenameIssueChosenSide getChosenSide() const;
    MoveOrRenameIssueChosenSide getSyncIdChosenSide() const;

    QString syncName() const;

    void increaseCombinedNumberOfIssues();
    int combinedNumberOfIssues() const;

    static bool findIssue(const std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue);

    void finishAsyncIssueSolving() override;

    bool solveAttemptsAchieved() const;

signals:
    void issueBeingSolved();

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    void onUndoFinished(std::shared_ptr<SyncSettings> syncSettings);

    bool mSolvingStarted;

    bool solveIssueByPathProblem(StalledIssueSPtr issue);
    bool solveSourceWasMovedToElsewhere(StalledIssueSPtr issue);
    bool solveParentFolderDoesNotExist(StalledIssueSPtr issue);
    bool solveDestinationPathInUnresolvedArea(StalledIssueSPtr issue);
    bool solveRemoteGenericIssues(StalledIssueSPtr issue);
    bool solveLocalGenericIssues(StalledIssueSPtr issue);

    bool isSourceWasDeleted(StalledIssueSPtr issue);
    bool areInTheSameDirectory(StalledIssueSPtr issue);

    ChoosableSides calculateChoosableSidesByPathProblem(StalledIssueSPtr issue);

    static QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> mChosenSideBySyncId;
    MoveOrRenameIssueChosenSide mChosenSide;
    int mCombinedNumberOfIssues;
    // Not in used for the moment
    QSet<QString> mFailedLocalPaths;
    QMap<mega::MegaHandle, std::shared_ptr<mega::MegaNode>> mFailedRemotePaths;
    //

    int mUndoSuccessful;
    uint mSolveAttempts;

    StalledIssuesList mDetectedCloudSideIssuesToFix;
    StalledIssuesList mDetectedLocalSideIssuesToFix;
};

class MoveOrRenameCannotOccurFactory : public StalledIssuesFactory
{
public:
    MoveOrRenameCannotOccurFactory(){}
    ~MoveOrRenameCannotOccurFactory() = default;

    StalledIssueSPtr createIssue(MultiStepIssueSolverBase* solver,
                                 const mega::MegaSyncStall* stall) override;
    void clear() override;

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
        mSyncId = issue->firstSyncId();
    }

    ~MoveOrRenameMultiStepIssueSolver() override{}

    bool checkIssue(const mega::MegaSyncStall* stall) const override
    {
        if(stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
        {
            auto syncIds(StalledIssuesBySyncFilter::getSyncIdsByStall(stall));
            return syncIds.contains(mSyncId);
        }
        return false;
    }

     void resetDeadlineIfNeeded(const StalledIssueVariant& issue) override
    {
        auto issueType = issue.convert<MoveOrRenameCannotOccurIssue>();
        if(issueType)
        {
            if(issueType->solveAttemptsAchieved())
            {
                setFailed();
            }
            else
            {
                MultiStepIssueSolver::resetDeadlineIfNeeded(issue);
            }
        }
    }

    mega::MegaHandle syncId() const {return mSyncId;}

private:
    mega::MegaHandle mSyncId = mega::INVALID_HANDLE;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
