#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{

}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{

}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("This file has changed since it it was last synced."));
}

//Special Files not supported
SpecialFilesNotSupportedHeader::SpecialFilesNotSupportedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
    connect(ui->actionButton, &QPushButton::clicked, this, &SpecialFilesNotSupportedHeader::on_actionButton_clicked);
}

void SpecialFilesNotSupportedHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot access <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("filesystem error preventing file access: Special files not supported"));

    ui->actionButton->show();
    ui->actionButton->setText(tr("Ignore"));
}

void SpecialFilesNotSupportedHeader::on_actionButton_clicked()
{
    auto data = getData().getStalledIssueData();
    if(data)
    {
        Preferences::instance()->setExcludedSyncPaths(QStringList() << data->mIndexPath);
        Preferences::instance()->setCrashed(true);

        QStringList exclusionPaths = Preferences::instance()->getExcludedSyncPaths();
        std::vector<std::string> vExclusionPaths;
        for (int i = 0; i < exclusionPaths.size(); i++)
        {
            vExclusionPaths.push_back(exclusionPaths[i].toUtf8().constData());
        }
        MegaSyncApp->getMegaApi()->setLegacyExcludedPaths(&vExclusionPaths);
    }
}

//Local folder not scannable
LocalFolderNotScannableHeader::LocalFolderNotScannableHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void LocalFolderNotScannableHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot access <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("filesystem error preventing folder scanning"));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void DeleteWaitingOnMovesHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("waiting to move <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("waiting for other processes to complete"));
}

//ApplyMoveNeedsOtherSideParentFolderToExist
ApplyMoveNeedsOtherSideParentFolderToExistHeader::ApplyMoveNeedsOtherSideParentFolderToExistHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void ApplyMoveNeedsOtherSideParentFolderToExistHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t move <b>%1</b> to the selected location").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("Cannot reach the destination folder on the cloud"));
}

