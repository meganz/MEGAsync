#include "StalledIssueHeader.h"
#include "ui_StalledIssueHeader.h"

#include "Utilities.h"

StalledIssueHeader::StalledIssueHeader(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader)
{
    ui->setupUi(this);
}

StalledIssueHeader::~StalledIssueHeader()
{
    delete ui;
}

void StalledIssueHeader::refreshUi()
{
    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                       getData()->mFileName, QLatin1Literal(":/images/drag_")));
    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));

    refreshUiByStalledReason();
}

void StalledIssueHeader::refreshUiByStalledReason()
{
    switch(getData()->mReason)
    {
    case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
    {
        //ui->actionButton->hide();

        auto errorTitleIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/ico_menu_full.png"));
        ui->errorTitleIcon->setPixmap(errorTitleIcon.pixmap(ui->errorTitleIcon->size()));
        ui->errorTitleText->setText(tr("Can´t sync <b>%1</b>").arg(getData()->mFileName));

        auto errorDescriptionIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/ico_info_hover.png"));
        ui->errorDescriptionIcon->setPixmap(errorDescriptionIcon.pixmap(ui->errorDescriptionIcon->size()));
        ui->errorDescriptionText->setText(tr("This file has conflicting copies"));
    }
    }
}
