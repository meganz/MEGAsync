#include "LocalAndRemoteNameConflicts.h"
#include "ui_LocalAndRemoteNameConflicts.h"

#include <StalledIssueHeader.h>

LocalAndRemoteNameConflicts::LocalAndRemoteNameConflicts(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::LocalAndRemoteNameConflicts)
{
    ui->setupUi(this);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::ICON_INDENT);

    connect(ui->cloudConflictNames, &NameConflict::actionFinished, this,
            [this](){
        updateIssues();
    });

    connect(ui->localConflictNames, &NameConflict::actionFinished, this,
            [this](){
        updateIssues();
    });
}

LocalAndRemoteNameConflicts::~LocalAndRemoteNameConflicts()
{
    delete ui;
}

void LocalAndRemoteNameConflicts::refreshUi()
{
    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(getData().data()))
    {
        auto cloudData = nameConflict->getNameConflictCloudData();
        cloudData.isEmpty() ? ui->cloudConflictNames->hide() : ui->cloudConflictNames->updateUi(cloudData);

        auto localData = nameConflict->getNameConflictLocalData();
        localData.isEmpty() ? ui->localConflictNames->hide() : ui->localConflictNames->updateUi(localData);
    }
}
