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

class MoveOrRenameCannotOccurFactory : public StalledIssuesFactory
{
public:
    struct SyncIssuesBeingSolved
    {
        SyncIssuesBeingSolved(mega::MegaHandle newSyncId = mega::INVALID_HANDLE, MoveOrRenameIssueChosenSide newChosenSide = MoveOrRenameIssueChosenSide::NONE)
            : syncId(newSyncId),
            chosenSide(newChosenSide)
        {}

        mega::MegaHandle syncId;
        MoveOrRenameIssueChosenSide chosenSide;
    };

    MoveOrRenameCannotOccurFactory(){}
    ~MoveOrRenameCannotOccurFactory() = default;

    StalledIssueVariant createIssue(const mega::MegaSyncStall* stall) override;
    void clear() override { mIssueBySyncId.clear(); }

    void setSyncIdIssuesBeingSolved(mega::MegaHandle syncId, MoveOrRenameIssueChosenSide side);

private:
    QHash<mega::MegaHandle, StalledIssueVariant> mIssueBySyncId;
    SyncIssuesBeingSolved mSyncIssuesBeingSolved;
};

class MoveOrRenameCannotOccurIssue : public QObject, public StalledIssue
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

    static bool findIssue(const std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue);
    static void solvingIssueInSeveralStepsFinished();

signals:
    void issueSolved(bool isSolved);

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    bool mSolvingStarted;
    std::shared_ptr<SyncController> mSyncController;
    static QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> mChosenSideBySyncId;
    MoveOrRenameIssueChosenSide mChosenSide;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
