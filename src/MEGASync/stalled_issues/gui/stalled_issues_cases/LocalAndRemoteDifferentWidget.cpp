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

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::ICON_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    auto issue = getData();

    if(issue.consultLocalData())
    {
        ui->chooseLocalCopy->setData(issue.consultLocalData());
    }

    if(issue.consultCloudData())
    {
        ui->chooseRemoteCopy->setData(issue.consultCloudData());
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{ 
    mUtilities.removeRemoteFile(ui->chooseRemoteCopy->data()->getFilePath());
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    mUtilities.removeLocalFile(ui->chooseRemoteCopy->data()->getNativeFilePath());
}
