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

    for(int index = 0; index < issue.stalledIssuesCount(); ++index)
    {
        auto data = issue.getStalledIssueData(index);

        if(data->mIsCloud)
        {
            ui->remotePath->show();
            ui->remotePath->updateUi(data, issue.getFileName());
        }
        else
        {
            ui->localPath->show();
            ui->localPath->updateUi(data, issue.getFileName());
        }
    }
}
