#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"

#include "mega/types.h"

#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::LocalAndRemoteDifferentWidget)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->chooseLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::BODY_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    auto issue = getData();

    if(issue.consultData()->consultLocalData())
    {
        ui->chooseLocalCopy->setIssueSolved(issue.consultData()->isSolved());
        ui->chooseLocalCopy->setData(issue.consultData()->consultLocalData());
    }

    if(issue.consultData()->consultCloudData())
    {
        ui->chooseRemoteCopy->setIssueSolved(issue.consultData()->isSolved());
        ui->chooseRemoteCopy->setData(issue.consultData()->consultCloudData());
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{ 
    mUtilities.removeRemoteFile(ui->chooseRemoteCopy->data()->getFilePath());
    MegaSyncApp->getStalledIssuesModel()->solveIssue(false, getCurrentIndex());

    refreshUi();
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    mUtilities.removeLocalFile(ui->chooseLocalCopy->data()->getNativeFilePath());
    MegaSyncApp->getStalledIssuesModel()->solveIssue(true, getCurrentIndex());

    refreshUi();
}
