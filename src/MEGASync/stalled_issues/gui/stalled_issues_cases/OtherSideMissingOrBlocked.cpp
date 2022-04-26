#include "OtherSideMissingOrBlocked.h"
#include "ui_OtherSideMissingOrBlocked.h"

OtherSideMissingOrBlocked::OtherSideMissingOrBlocked(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::OtherSideMissingOrBlocked)
{
    ui->setupUi(this);
}

OtherSideMissingOrBlocked::~OtherSideMissingOrBlocked()
{
    delete ui;
}

void OtherSideMissingOrBlocked::refreshUi()
{
    auto issue = getData();
    for(int index = 0; index < issue.stalledIssuesCount(); ++index)
    {
        auto data = issue.getStalledIssueData(index);
        data->mIsCloud ? ui->remotePath->updateUi(getCurrentIndex(), data) : ui->localPath->updateUi(getCurrentIndex(), data);
    }
}
