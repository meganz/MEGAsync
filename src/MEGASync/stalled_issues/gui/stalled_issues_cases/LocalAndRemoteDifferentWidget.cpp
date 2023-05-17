#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"

#include "mega/types.h"

#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> originalstall, QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    originalStall(originalstall),
    ui(new Ui::LocalAndRemoteDifferentWidget)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
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
        ui->chooseLocalCopy->updateUi(issue.consultData()->consultLocalData());
    }

    if(issue.consultData()->consultCloudData())
    {
        ui->chooseRemoteCopy->setIssueSolved(issue.consultData()->isSolved());
        ui->chooseRemoteCopy->updateUi(issue.consultData()->consultCloudData());
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{
    mUtilities.removeRemoteFile(ui->chooseRemoteCopy->data()->getFilePath());
    MegaSyncApp->getStalledIssuesModel()->solveIssue(false, getCurrentIndex());

    // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
    MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

    refreshUi();
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    mUtilities.removeLocalFile(ui->chooseLocalCopy->data()->getNativeFilePath());
    MegaSyncApp->getStalledIssuesModel()->solveIssue(true, getCurrentIndex());

    // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
    MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

    refreshUi();
}
