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

#endif // STALLEDISSUESCASEHEADERS_H
