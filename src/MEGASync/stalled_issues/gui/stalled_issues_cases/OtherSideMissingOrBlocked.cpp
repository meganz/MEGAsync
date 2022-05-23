#include "OtherSideMissingOrBlocked.h"
#include "ui_OtherSideMissingOrBlocked.h"

#include "StalledIssueHeader.h"

OtherSideMissingOrBlocked::OtherSideMissingOrBlocked(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::OtherSideMissingOrBlocked)
{
    ui->setupUi(this);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->localPath->setIndent(StalledIssueHeader::ICON_INDENT);
    ui->remotePath->setIndent(StalledIssueHeader::ICON_INDENT);
}

OtherSideMissingOrBlocked::~OtherSideMissingOrBlocked()
{
    delete ui;
}

void OtherSideMissingOrBlocked::refreshUi()
{
    auto issue = getData();

    ui->localPath->hide();
    ui->remotePath->hide();

    auto localData = issue.consultData()->consultLocalData();
    auto cloudData = issue.consultData()->consultCloudData();

    if(cloudData)
    {
        ui->remotePath->show();
        ui->remotePath->updateUi(cloudData);
    }

    if(localData)
    {
        ui->localPath->show();
        ui->localPath->updateUi(localData);
    }
}
