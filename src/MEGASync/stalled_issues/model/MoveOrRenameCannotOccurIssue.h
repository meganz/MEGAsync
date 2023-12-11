#ifndef MOVEORRENAMECANNOTOCCURISSUE_H
#define MOVEORRENAMECANNOTOCCURISSUE_H

#include <StalledIssue.h>

class SyncController;
class SyncSettings;

class MoveOrRenameCannotOccurIssue : public QObject, public StalledIssue
{
    Q_OBJECT

public:
    MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall *stall);

    void fillIssue(const mega::MegaSyncStall *stall) override;

    bool isSolvable() const override;
    void solveIssue();

    bool checkForExternalChanges() override;

    const QString &currentPath() const;
    const QString& previousPath() const;

signals:
    void issueSolved(bool isSolved);

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    struct PathToSolveInfo
    {
        bool isCloud;

        QString currentPath;
        QString previousPath;
        mega::MegaHandle currentHandle = mega::INVALID_HANDLE;
        mega::MegaHandle previousHandle = mega::INVALID_HANDLE;
    };
    PathToSolveInfo mPathToSolve;

    bool mSolvingStarted;
    std::shared_ptr<SyncController> mSyncController;
    bool mIsSolvable;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
