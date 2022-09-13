#include "LocalAndRemoteNameConflicts.h"
#include "ui_LocalAndRemoteNameConflicts.h"

#include <StalledIssueHeader.h>

const QString LocalAndRemoteNameConflicts::FILES_DESCRIPTION = QString::fromLatin1(QT_TR_NOOP("Renaming or removing files can resolve this issue,"
                                                                                                     "\nor click the Folders below to make adjustments in the local filesystem or in MEGA"));
const QString LocalAndRemoteNameConflicts::FOLDERS_DESCRIPTION = QString::fromLatin1(QT_TR_NOOP("Renaming or removing folders can resolve this issue,"
                                                                                                       "\nor click the Folders below to make adjustments in the local filesystem or in MEGA"));
const QString LocalAndRemoteNameConflicts::FILES_AND_FOLDERS_DESCRIPTION = QString::fromLatin1(QT_TR_NOOP("Renaming or removing files or folders can resolve this issue,"
                                                                                                       "\nor click the Folders below to make adjustments in the local filesystem or in MEGA"));

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

    connect(ui->cloudConflictNames, &NameConflict::allSolved, this, &LocalAndRemoteNameConflicts::onNameConflictSolved);
    connect(ui->localConflictNames, &NameConflict::allSolved, this, &LocalAndRemoteNameConflicts::onNameConflictSolved);
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

        if(getData().consultData()->hasFiles() > 0 && getData().consultData()->hasFolders() > 0)
        {
            ui->selectLabel->setText(FILES_AND_FOLDERS_DESCRIPTION);
        }
        else if(getData().consultData()->hasFiles() > 0)
        {
            ui->selectLabel->setText(FILES_DESCRIPTION);
        }
        else if(getData().consultData()->hasFolders() > 0)
        {
            ui->selectLabel->setText(FOLDERS_DESCRIPTION);
        }

        update();
    }
}

void LocalAndRemoteNameConflicts::onNameConflictSolved()
{
    ui->cloudConflictNames->setSolved();
    ui->localConflictNames->setSolved();
}
