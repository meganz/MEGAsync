#include "LocalAndRemoteNameConflicts.h"
#include "ui_LocalAndRemoteNameConflicts.h"

#include <StalledIssueHeader.h>

LocalAndRemoteNameConflicts::LocalAndRemoteNameConflicts(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::LocalAndRemoteNameConflicts)
{
    ui->setupUi(this);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    connect(ui->cloudConflictNames, &NameConflict::refreshUi, this, &LocalAndRemoteNameConflicts::refreshUi);
    connect(ui->localConflictNames, &NameConflict::refreshUi, this, &LocalAndRemoteNameConflicts::refreshUi);
}

LocalAndRemoteNameConflicts::~LocalAndRemoteNameConflicts()
{
    delete ui;
}

void LocalAndRemoteNameConflicts::refreshUi()
{
    if(auto nameConflict = std::dynamic_pointer_cast<const NameConflictedStalledIssue>(getData().consultData()))
    {
        auto cloudData = nameConflict->getNameConflictCloudData();
        if(cloudData.isEmpty())
        {
            ui->cloudConflictNames->hide();
        }
        else
        {
            ui->cloudConflictNames->updateUi(cloudData);
            ui->cloudConflictNames->show();
        }

        auto localData = nameConflict->getNameConflictLocalData();
        if(localData.isEmpty())
        {
            ui->localConflictNames->hide();
        }
        else
        {
            ui->localConflictNames->updateUi(localData);
            ui->localConflictNames->show();
        }

        update();
    }
}
