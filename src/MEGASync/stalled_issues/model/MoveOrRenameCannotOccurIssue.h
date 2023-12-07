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

    bool isSolvable() const;
    void solveIssue();

    bool checkForExternalChanges() override;

    const QString& pathToCreate() const;

signals:
    void issueSolved(bool isSolved);

private slots:
    void onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings);

private:
    struct PathToCreateInfo
    {
        bool isCloud;
        QString path;
    };
    PathToCreateInfo mPathToCreate;

    bool mSolvingStarted;
    std::shared_ptr<SyncController> mSyncController;
};

#endif // MOVEORRENAMECANNOTOCCURISSUE_H
