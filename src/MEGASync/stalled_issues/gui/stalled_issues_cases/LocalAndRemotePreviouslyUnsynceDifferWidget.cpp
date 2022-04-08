#include "LocalAndRemotePreviouslyUnsynceDifferWidget.h"
#include "ui_LocalAndRemotePreviouslyUnsynceDifferWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"

#include <QFile>

LocalAndRemotePreviouslyUnsynceDifferWidget::LocalAndRemotePreviouslyUnsynceDifferWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::LocalAndRemotePreviouslyUnsynceDifferWidget)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemotePreviouslyUnsynceDifferWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemotePreviouslyUnsynceDifferWidget::onRemoteButtonClicked);
}

LocalAndRemotePreviouslyUnsynceDifferWidget::~LocalAndRemotePreviouslyUnsynceDifferWidget()
{
    delete ui;
}

void LocalAndRemotePreviouslyUnsynceDifferWidget::refreshUi()
{
    auto issue = getData();
    for(int index = 0; index < issue.stalledIssuesCount(); ++index)
    {
        auto data = issue.getStalledIssueData(index);
        data->mIsCloud ? ui->chooseRemoteCopy->setData(data, issue.getFileName()) : ui->chooseLocalCopy->setData(data, issue.getFileName());
    }
}

void LocalAndRemotePreviouslyUnsynceDifferWidget::onLocalButtonClicked()
{
    auto fileNode = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseLocalCopy->data()->mCloudPath.toStdString().c_str()));
    if(fileNode)
    {
        MegaSyncApp->getMegaApi()->remove(fileNode.get());
        MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
    }
}

void LocalAndRemotePreviouslyUnsynceDifferWidget::onRemoteButtonClicked()
{
    QFile file(ui->chooseLocalCopy->data()->mIndexPath);
    if(file.exists())
    {
        if(file.remove())
        {
            MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
        }
    }
}
