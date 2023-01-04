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

const char* TITLE_FILENAME = "TITLE_FILENAME";

//NAME CONFLICT TITLE
//THINK ABOUT ANOTHER WAY TO RESUSE THE STALLED ISSUE ACTION TITLE OR CREATE ONE FOR THE NAME CONFLICT
NameConflictTitle::NameConflictTitle(int index, QWidget *parent)
    : mIndex(index),
      StalledIssueActionTitle(parent)
{
    initTitle();
}

void NameConflictTitle::initTitle()
{
    QIcon renameIcon(QString::fromUtf8("://images/StalledIssues/rename_node_default.png"));
    QIcon removeIcon(QString::fromUtf8("://images/StalledIssues/remove_default.png"));
    addActionButton(renameIcon, tr("Rename"), RENAME_ID, false);
    addActionButton(removeIcon, QString(), REMOVE_ID, false);
}

int NameConflictTitle::getIndex() const
{
    return mIndex;
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

void NameConflict::updateUi(NameConflictedStalledIssue::NameConflictData conflictData)
{
    ui->path->hide();

    if(conflictData.data)
    {
        ui->path->show();
        ui->path->updateUi(conflictData.data);
    }

    //Fill conflict names
    auto conflictedNames = conflictData.conflictedNames;
    auto nameConflictWidgets = ui->nameConflicts->findChildren<NameConflictTitle*>();
    auto totalUpdate(nameConflictWidgets.size() >= conflictedNames.size());

    bool allSolved(true);

    for(int index = 0; index < conflictedNames.size(); ++index)
    {
        QString conflictedName(conflictedNames.at(index).conflictedName);

        NameConflictTitle* title(nullptr);
        if(totalUpdate)
        {
            title = nameConflictWidgets.first();
            nameConflictWidgets.removeFirst();
        }
        else
        {
            title = new NameConflictTitle(index, this);
            title->setProperty(TITLE_FILENAME, conflictedName);
            title->setSolved(false);
            connect(title, &StalledIssueActionTitle::actionClicked, this, &NameConflict::onActionClicked);

            ui->nameConflictsLayout->addWidget(title);
        }

        mData.data->checkTrailingSpaces(conflictedName);
        title->setTitle(conflictedName);
        title->showIcon();

        if(title &&
                (!mData.data
                 || (mData.conflictedNames.size() > index
                 && (conflictData.conflictedNames.at(index).isSolved() != mData.conflictedNames.at(index).isSolved()))))
        {
            bool isSolved(conflictedNames.at(index).isSolved());
            title->setSolved(isSolved);
            if(isSolved)
            {
                title->hideActionButton(RENAME_ID);
                title->hideActionButton(REMOVE_ID);

                QIcon icon;
                QString titleText;

                if(conflictedNames.at(index).solved ==  NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE)
                {
                    icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                    titleText = tr("Removed");
                }
                else if(conflictedNames.at(index).solved ==  NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME)
                {
                    icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                    titleText = tr("Renamed to \"%1\"").arg(conflictedNames.at(index).renameTo);
                }
                else
                {
                    icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                    titleText = tr("No action needed");
                }

                title->addMessage(titleText, icon.pixmap(24,24));
            }
        }

        allSolved &= conflictedNames.at(index).isSolved();
    }

    if(allSolved)
    {
        setSolved();
    }

    //No longer exist conflicts
    foreach(auto titleToRemove, nameConflictWidgets)
    {
        removeConflictedNameWidget(titleToRemove);
    }

    mData = conflictData;
}

void NameConflict::removeConflictedNameWidget(QWidget* widget)
{
    ui->nameConflictsLayout->removeWidget(widget);
    widget->deleteLater();
}

void NameConflict::setSolved()
{
    if(!ui->pathContainer->graphicsEffect())
    {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.30);
        ui->pathContainer->setGraphicsEffect(effect);
    }
}

void NameConflict::onActionClicked(int actionId)
{
    if(auto chooseTitle = dynamic_cast<NameConflictTitle*>(sender()))
    {
        QFileInfo info;
        auto titleFileName = chooseTitle->property(TITLE_FILENAME).toString();
        info.setFile(mData.data->getNativePath(), titleFileName);

        auto delegateWidget = dynamic_cast<StalledIssueBaseDelegateWidget*>(parentWidget());

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
                bool areAllSolved(false);

                if(mData.isCloud)
                {
                   areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRename(titleFileName
                                                                                           , newName, chooseTitle->getIndex(), delegateWidget->getCurrentIndex());
                }
                else
                {
                    areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRename(titleFileName
                                                                                           , newName,chooseTitle->getIndex(), delegateWidget->getCurrentIndex());
                }

                if(areAllSolved)
                {
                    emit allSolved();
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
                bool areAllSolved(false);

                if(mData.isCloud)
                {
                    mUtilities.removeRemoteFile(info.filePath());
                    areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRemove(titleFileName,chooseTitle->getIndex(), delegateWidget->getCurrentIndex());
                }
                else
                {
                    mUtilities.removeLocalFile(QDir::toNativeSeparators(info.filePath()));
                    areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRemove(titleFileName,chooseTitle->getIndex(), delegateWidget->getCurrentIndex());
                }

                if(areAllSolved)
                {
                    emit allSolved();
                }
            }
        }

        //Now, close the editor because the action has been finished
        if(delegateWidget)
        {
            emit refreshUi();
        }
    }
}
