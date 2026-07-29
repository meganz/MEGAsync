#ifndef MOVEORRENAMECANNOTOCCURISSUE_H
#define MOVEORRENAMECANNOTOCCURISSUE_H

#include "StalledIssue.h"
#include "StalledIssuesFactory.h"
#include "StalledIssuesUtilities.h"

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

    void setIsSolved(ResolutionState type) override;
    bool isAutoSolvable() const override;
    void solveIssue(MoveOrRenameIssueChosenSide side);
    ResolutionState autoSolveIssue() override;

    bool isKeepSideAvailable(MoveOrRenameIssueChosenSide side) const;

    bool checkForExternalChanges(QObject* context) override;

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
    // Guards against onSyncPausedEnds() being re-entered by the nested event loop that the
    // synchronous SDK requests spin while the undo is running.
    bool mUndoInProgress;

    bool solveIssueByPathProblem(StalledIssueSPtr issue);
    bool solveSourceWasMovedToElsewhere(StalledIssueSPtr issue);
    bool solveParentFolderDoesNotExist(StalledIssueSPtr issue);
    bool solveDestinationPathInUnresolvedArea(StalledIssueSPtr issue);
    bool solveRemoteGenericIssues(StalledIssueSPtr issue);
    bool solveLocalGenericIssues(StalledIssueSPtr issue);

    bool wasSourceDeleted(StalledIssueSPtr issue) const;
    bool areInTheSameDirectory(StalledIssueSPtr issue);

    bool remoteSideWasChosen() const;
    bool localSideWasChosen() const;

    ChoosableSides calculateChoosableSidesByPathProblem(StalledIssueSPtr issue);

    static QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> mChosenSideBySyncId;
    MoveOrRenameIssueChosenSide mChosenSide;
    int mCombinedNumberOfIssues;
    // Not in used for the moment
    QSet<QString> mFailedLocalPaths;
    QMap<mega::MegaHandle, std::shared_ptr<mega::MegaNode>> mFailedRemotePaths;
    //

    qsizetype mUndoSuccessful;
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
    void finish() override;

    QSet<mega::MegaHandle> backupSyncsDetected() const;

private:
    QHash<mega::MegaHandle, std::shared_ptr<MoveOrRenameCannotOccurIssue>> mIssueBySyncId;
    QSet<mega::MegaHandle> mBackupSyncsDetected;
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

    bool checkIssue(const mega::MegaSyncStall* stall, mega::MegaHandle syncId) const override
    {
        if(stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
        {
            return mSyncId == syncId;
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
