#include "OtherSideMissingOrBlocked.h"
#include "ui_OtherSideMissingOrBlocked.h"

#include "StalledIssueHeader.h"

#include <QGraphicsOpacityEffect>

OtherSideMissingOrBlocked::OtherSideMissingOrBlocked(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::OtherSideMissingOrBlocked)
{
    ui->setupUi(this);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->localPath->setIndent(StalledIssueHeader::ICON_INDENT);
    ui->localPath->showFullPath();
    ui->remotePath->setIndent(StalledIssueHeader::ICON_INDENT);
    ui->remotePath->showFullPath();
}

OtherSideMissingOrBlocked::~OtherSideMissingOrBlocked()
{
    delete ui;
}

void OtherSideMissingOrBlocked::refreshUi()
{
    auto issue = getData();

    auto localData = issue.consultData()->consultLocalData();
    auto cloudData = issue.consultData()->consultCloudData();

    if(cloudData)
    {
        ui->remotePath->show();
        ui->remotePath->updateUi(cloudData);
    }
    else
    {
        ui->remotePath->hide();
    }

    if(localData)
    {
        ui->localPath->show();
        ui->localPath->updateUi(localData);
    }
    else
    {
        ui->localPath->hide();
    }

    if(issue.consultData()->isSolved())
    {
        if(!graphicsEffect())
        {
            auto effect = new QGraphicsOpacityEffect(this);
            effect->setOpacity(0.30);
            setGraphicsEffect(effect);
        }
    }
    else if(graphicsEffect())
    {
        graphicsEffect()->deleteLater();
        setGraphicsEffect(nullptr);
    }
}
