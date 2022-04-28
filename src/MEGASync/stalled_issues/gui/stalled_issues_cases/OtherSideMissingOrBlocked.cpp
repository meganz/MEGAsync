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

    Q_ASSERT(issue.stalledIssuesCount() == 2);

    if(issue.stalledIssuesCount() == 2)
    {
        auto mainIssue = issue.getStalledIssueData();
        bool isCloud(issue.getStalledIssueData()->mIsCloud);

        for(int index = 0; index < issue.stalledIssuesCount(); ++index)
        {
            auto data = issue.getStalledIssueData(index);
            data->mIsCloud ? ui->remotePath->updateUi(getCurrentIndex(), data) : ui->localPath->updateUi(getCurrentIndex(), data);
        }
    }
}
