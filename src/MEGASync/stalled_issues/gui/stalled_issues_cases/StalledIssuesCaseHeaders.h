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

#endif // STALLEDISSUESCASEHEADERS_H
