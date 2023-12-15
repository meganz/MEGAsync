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
    bool refreshListAfterSolving() const override;
    void solveIssue();

    bool checkForExternalChanges() override;

    const QString &currentPath() const;
    QString previousPath() const;

    bool isFile() const;

signals:
    void issueSolved(bool isSolved);

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    struct
    {
        bool isCloud = false;
        bool isFile = false;

        QString currentPath;
        QString previousPath;
        mega::MegaHandle currentHandle = mega::INVALID_HANDLE;
        mega::MegaHandle previousHandle = mega::INVALID_HANDLE;
    } mPathToSolve;

    bool mSolvingStarted;
    std::shared_ptr<SyncController> mSyncController;
    bool mIsSolvable;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
