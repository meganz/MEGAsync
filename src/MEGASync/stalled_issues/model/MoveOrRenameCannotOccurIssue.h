#ifndef MOVEORRENAMECANNOTOCCURISSUE_H
#define MOVEORRENAMECANNOTOCCURISSUE_H

#include <StalledIssue.h>
#include <StalledIssuesFactory.h>

class SyncController;
class SyncSettings;

class MoveOrRenameCannotOccurFactory : public StalledIssuesFactory
{
public:
    MoveOrRenameCannotOccurFactory() = default;
    ~MoveOrRenameCannotOccurFactory() = default;

    StalledIssueVariant createIssue(const mega::MegaSyncStall* stall) override;
    void clear() override { mIssueBySyncId.clear(); }

private:
    QHash<mega::MegaHandle, StalledIssueVariant> mIssueBySyncId;
};

class MoveOrRenameCannotOccurIssue : public QObject, public StalledIssue
{
    Q_OBJECT

public:
    enum class SideChosen
    {
        NONE,
        LOCAL,
        REMOTE,
    };

    MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall *stall);

    void fillIssue(const mega::MegaSyncStall*) override;
    void fillCloudSide(const mega::MegaSyncStall* stall);
    void fillLocalSide(const mega::MegaSyncStall* stall);

    bool isSolvable() const override;
    bool refreshListAfterSolving() const override;
    void solveIssue(SideChosen side);

    bool checkForExternalChanges() override;

signals:
    void issueSolved(bool isSolved);

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    bool mSolvingStarted;
    std::shared_ptr<SyncController> mSyncController;
    SideChosen mSideChosen;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
