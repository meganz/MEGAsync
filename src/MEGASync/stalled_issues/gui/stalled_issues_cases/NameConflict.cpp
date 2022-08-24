#include "NameConflict.h"
#include "ui_NameConflict.h"

#include "StalledIssueHeader.h"

#include <MegaApplication.h>
#include <StalledIssuesModel.h>
#include "NodeNameSetterDialog/RenameNodeDialog.h"
#include "QMegaMessageBox.h"

#include <QDialogButtonBox>

static const int RENAME_ID = 0;
static const int REMOVE_ID = 1;

//NAME CONFLICT TITLE
//THINK ABOUT ANOTHER WAY TO RESUSE THE STALLED ISSUE ACTION TITLE OR CREATE ONE FOR THE NAME CONFLICT
NameConflictTitle::NameConflictTitle(QWidget *parent)
    : StalledIssueActionTitle(parent)
{
    initTitle();
}

void NameConflictTitle::initTitle()
{
    setRoundedCorners(StalledIssueActionTitle::RoundedCorners::ALL_CORNERS);
    QIcon renameIcon(QString::fromUtf8("://images/StalledIssues/rename_node_default.png"));
    QIcon removeIcon(QString::fromUtf8("://images/StalledIssues/remove_default.png"));
    addActionButton(renameIcon, tr("Rename"), RENAME_ID, false);
    addActionButton(removeIcon, QString(), REMOVE_ID, false);
}

//NAME CONFLICT
NameConflict::NameConflict(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameConflict)
{
    ui->setupUi(this);
}

NameConflict::~NameConflict()
{
    delete ui;
}

void NameConflict::updateUi(NameConflictedStalledIssue::NameConflictData data)
{
    ui->path->hide();

    if(data.data)
    {
        ui->path->show();
        ui->path->updateUi(data.data);
    }

    //Fill conflict names
    auto conflictedNames = data.conflictedNames;
    auto nameConflictWidgets = ui->nameConflicts->findChildren<StalledIssueActionTitle*>();
    auto totalUpdate(nameConflictWidgets.size() >= conflictedNames.size());

    for(int index = 0; index < conflictedNames.size(); ++index)
    {
        StalledIssueActionTitle* title(nullptr);
        if(totalUpdate)
        {
            title = nameConflictWidgets.first();
            nameConflictWidgets.removeFirst();
        }
        else
        {
            title = new NameConflictTitle(this);
            connect(title, &StalledIssueActionTitle::actionClicked, this, &NameConflict::onActionClicked);

            ui->nameConflictsLayout->addWidget(title);
        }

        title->setTitle(conflictedNames.at(index).conflictedName);
        title->showIcon();

        if(title &&
                (!mData.data
                 || (mData.conflictedNames.size() > index
                 && (data.conflictedNames.at(index).isSolved() != mData.conflictedNames.at(index).isSolved()))))
        {
            bool isSolved(conflictedNames.at(index).isSolved());
            title->setDisabled(isSolved);
            if(isSolved)
            {
                title->hideActionButton(RENAME_ID);
                title->hideActionButton(REMOVE_ID);

                if(conflictedNames.at(index).solved ==  NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE)
                {
                    title->addMessage(tr("REMOVED"));
                }
                else
                {
                    title->addMessage(tr("RENAME TO %1").arg(conflictedNames.at(index).renameTo));
                }
            }
        }
    }

    //No longer exist conflicts
    foreach(auto titleToRemove, nameConflictWidgets)
    {
        removeConflictedNameWidget(titleToRemove);
    }

    mData = data;
}

void NameConflict::removeConflictedNameWidget(QWidget* widget)
{
    ui->nameConflictsLayout->removeWidget(widget);
    widget->deleteLater();
}

void NameConflict::onActionClicked(int actionId)
{
    if(auto chooseTitle = dynamic_cast<StalledIssueActionTitle*>(sender()))
    {
        QFileInfo info;
        info.setFile(mData.data->getNativePath(), chooseTitle->title());

        auto delegateWidget = dynamic_cast<StalledIssueBaseDelegateWidget*>(parentWidget());

        //As a dialog will be shown, avoid removing the editor while the dialog is open
        if(delegateWidget)
        {
            delegateWidget->setKeepEditor(true);
        }

        if(actionId == RENAME_ID)
        {
            QPointer<NameConflict> currentWidget = QPointer<NameConflict>(this);

            QString newName;
            bool result(QDialog::Rejected);

            if(mData.isCloud)
            {
                RenameRemoteNodeDialog dialog(info.filePath(), nullptr);
                result = dialog.show();
                newName = dialog.getName();
            }
            else
            {
                RenameLocalNodeDialog dialog(info.filePath(), nullptr);
                result = dialog.show();
                newName = dialog.getName();
            }

            if(result == QDialog::Accepted)
            {
                if(mData.isCloud)
                {
                    MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRename(chooseTitle->title()
                                                                                           , newName, delegateWidget->getCurrentIndex());
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRename(chooseTitle->title()
                                                                                           , newName, delegateWidget->getCurrentIndex());
                }

                chooseTitle->setDisabled(true);
            }

            if(!currentWidget)
            {
                return;
            }
        }
        else if(actionId == REMOVE_ID)
        {
            QPointer<NameConflict> currentWidget = QPointer<NameConflict>(this);

            if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                                     tr("Are you sure you want to remove the %1 %2 %3?")
                                     .arg(mData.isCloud ? tr("remote") : tr("local"))
                                     .arg(info.isFile() ? tr("file") : tr("folder"))
                                     .arg(info.fileName()),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                    == QMessageBox::Yes
                    && currentWidget)
            {
                if(mData.isCloud)
                {
                    mUtilities.removeRemoteFile(info.filePath());
                    MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRemove(chooseTitle->title(), delegateWidget->getCurrentIndex());
                }
                else
                {
                    mUtilities.removeLocalFile(QDir::toNativeSeparators(info.filePath()));
                    MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRemove(chooseTitle->title(), delegateWidget->getCurrentIndex());
                }
            }
        }

        //Now, close the editor beacuse the action has been finished
        if(delegateWidget)
        {
            emit refreshUi();
            delegateWidget->setKeepEditor(false);
        }
    }
}
