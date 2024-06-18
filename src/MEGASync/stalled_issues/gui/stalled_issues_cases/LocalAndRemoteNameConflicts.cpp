#include "LocalAndRemoteNameConflicts.h"
#include "ui_LocalAndRemoteNameConflicts.h"

#include <StalledIssueHeader.h>
#include <NameConflictStalledIssue.h>

const QString FILES_DESCRIPTION = QString::fromLatin1(QT_TRANSLATE_NOOP("LocalAndRemoteNameConflicts",
                                                                        "Click Rename to resolve this issue, or click the folders below to make adjustments in the local filesystem or in MEGA"));

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

    ui->cloudConflictNames->setDelegate(this);
    ui->localConflictNames->setDelegate(this);
}

LocalAndRemoteNameConflicts::~LocalAndRemoteNameConflicts()
{
    delete ui;
}

void LocalAndRemoteNameConflicts::refreshUi()
{
    if(auto nameConflict =  getData().convert<NameConflictedStalledIssue>())
    {
        auto cloudData = nameConflict->getNameConflictCloudData();
        if(cloudData.isEmpty())
        {
            ui->cloudConflictNames->hide();
        }
        else
        {
            ui->cloudConflictNames->updateUi(nameConflict);
            ui->cloudConflictNames->show();
        }

        auto localData = nameConflict->getNameConflictLocalData();
        if(localData.isEmpty())
        {
            ui->localConflictNames->hide();
        }
        else
        {
            ui->localConflictNames->updateUi(nameConflict);
            ui->localConflictNames->show();
        }
        ui->selectLabel->setText(FILES_DESCRIPTION);
    }
}
