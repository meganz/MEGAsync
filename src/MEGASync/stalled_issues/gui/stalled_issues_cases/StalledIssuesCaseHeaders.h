#ifndef STALLEDISSUESCASEHEADERS_H
#define STALLEDISSUESCASEHEADERS_H

#include <StalledIssueHeader.h>

//DefaultHeader failed
class DefaultHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    DefaultHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Create folder failed
class FileIssueHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    FileIssueHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Move or rename failed
class MoveOrRenameCannotOccurHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    MoveOrRenameCannotOccurHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Delete or Move Waiting onScanning
class DeleteOrMoveWaitingOnScanningHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    DeleteOrMoveWaitingOnScanningHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Delete waiting on moves
class DeleteWaitingOnMovesHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    DeleteWaitingOnMovesHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Upsync needs target folder
class UploadIssueHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    UploadIssueHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Downsync needs target folder
class DownloadIssueHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    DownloadIssueHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Create folder failed
class CannotCreateFolderHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    CannotCreateFolderHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Create folder failed
class CannotPerformDeletionHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    CannotPerformDeletionHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//SyncItemExceedsSupoortedTreeDepth
class SyncItemExceedsSupoortedTreeDepthHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    SyncItemExceedsSupoortedTreeDepthHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;

protected slots:
    void on_actionButton_clicked() override;
};


//Folder matched against file
class FolderMatchedAgainstFileHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    FolderMatchedAgainstFileHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

class LocalAndRemotePreviouslyUnsyncedDifferHeader : public StalledIssueHeader
{
public:
    LocalAndRemotePreviouslyUnsyncedDifferHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Local and remote previously synced differ
class LocalAndRemoteChangedSinceLastSyncedStateHeader : public StalledIssueHeader
{
public:
    LocalAndRemoteChangedSinceLastSyncedStateHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Local and remote previously synced differ
class NameConflictsHeader : public StalledIssueHeader
{
public:
    NameConflictsHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

#endif // STALLEDISSUESCASEHEADERS_H
