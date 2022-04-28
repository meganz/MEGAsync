#ifndef STALLEDISSUESCASEHEADERS_H
#define STALLEDISSUESCASEHEADERS_H

#include <StalledIssueHeader.h>

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

//Special Files not supported
class SpecialFilesNotSupportedHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    SpecialFilesNotSupportedHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;

protected slots:
    void on_actionButton_clicked();
};

//Local Folder not scanneable
class LocalFolderNotScannableHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    LocalFolderNotScannableHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Folder contains locked files
class FolderContainsLockedFilesHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    FolderContainsLockedFilesHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Can fingerprint file yet
class CantFingerprintFileYetHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    CantFingerprintFileYetHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Create folder failed
class CreateFolderFailedHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    CreateFolderFailedHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Could not moved to local debris
class CouldNotMoveToLocalDebrisFolderHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    CouldNotMoveToLocalDebrisFolderHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};


//Move or rename failed
class MoveOrRenameFailedHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    MoveOrRenameFailedHeader(QWidget *parent = nullptr);

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

//ApplyMoveNeedsOtherSideParentFolderToExist
class ApplyMoveNeedsOtherSideParentFolderToExistHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    ApplyMoveNeedsOtherSideParentFolderToExistHeader(QWidget *parent = nullptr);

protected:
    void refreshCaseUi() override;
};

//Unable to load ignore file
class UnableToLoadIgnoreFileHeader : public StalledIssueHeader
{
    Q_OBJECT

public:
    UnableToLoadIgnoreFileHeader(QWidget *parent = nullptr);

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
};

#endif // STALLEDISSUESCASEHEADERS_H
