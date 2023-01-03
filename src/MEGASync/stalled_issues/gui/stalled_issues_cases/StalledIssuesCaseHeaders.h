#ifndef STALLEDISSUESCASEHEADERS_H
#define STALLEDISSUESCASEHEADERS_H

#include <StalledIssueHeader.h>
#include <QPointer>

class StalledIssueHeaderCase : public QObject
{
    Q_OBJECT

public:
    StalledIssueHeaderCase(StalledIssueHeader* header);
    ~StalledIssueHeaderCase() = default;

    QPointer<StalledIssueHeader> getStalledIssueHeader();
    void reset();

protected:
    QPointer<StalledIssueHeader> mStalledIssueHeader;

protected slots:
    virtual void onRefreshCaseUi() = 0;
};

//DefaultHeader failed
class DefaultHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    DefaultHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Create folder failed
class FileIssueHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    FileIssueHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Move or rename failed
class MoveOrRenameCannotOccurHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    MoveOrRenameCannotOccurHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Delete or Move Waiting onScanning
class DeleteOrMoveWaitingOnScanningHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    DeleteOrMoveWaitingOnScanningHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Delete waiting on moves
class DeleteWaitingOnMovesHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    DeleteWaitingOnMovesHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Upsync needs target folder
class UploadIssueHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    UploadIssueHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Downsync needs target folder
class DownloadIssueHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    DownloadIssueHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Create folder failed
class CannotCreateFolderHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    CannotCreateFolderHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Create folder failed
class CannotPerformDeletionHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    CannotPerformDeletionHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//SyncItemExceedsSupoortedTreeDepth
class SyncItemExceedsSupoortedTreeDepthHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    SyncItemExceedsSupoortedTreeDepthHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};


//Folder matched against file
class FolderMatchedAgainstFileHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    FolderMatchedAgainstFileHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

class LocalAndRemotePreviouslyUnsyncedDifferHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    LocalAndRemotePreviouslyUnsyncedDifferHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Local and remote previously synced differ
class LocalAndRemoteChangedSinceLastSyncedStateHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    LocalAndRemoteChangedSinceLastSyncedStateHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

//Local and remote previously synced differ
class NameConflictsHeader : public StalledIssueHeaderCase
{
    Q_OBJECT

public:
    NameConflictsHeader(StalledIssueHeader* header);

protected slots:
    void onRefreshCaseUi() override;
};

#endif // STALLEDISSUESCASEHEADERS_H
