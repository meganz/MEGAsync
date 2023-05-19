#include "NameConflict.h"
#include "ui_NameConflict.h"

#include "StalledIssueHeader.h"

#include <MegaApplication.h>
#include <StalledIssuesModel.h>
#include "NodeNameSetterDialog/RenameNodeDialog.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include "StalledIssuesDialog.h"
#include "MegaNodeNames.h"
#include "DateTimeFormatter.h"
#include "StalledIssuesUtilities.h"

#include <megaapi.h>

#include <QDialogButtonBox>

static const int RENAME_ID = 0;
static const int REMOVE_ID = 1;

const char* TITLE_FILENAME = "TITLE_FILENAME";

//NAME CONFLICT TITLE
//THINK ABOUT ANOTHER WAY TO RESUSE THE STALLED ISSUE ACTION TITLE OR CREATE ONE FOR THE NAME CONFLICT
NameConflictTitle::NameConflictTitle(int index, const QString& conflictedName, QWidget *parent)
    : mIndex(index),
      StalledIssueActionTitle(parent)
{
    initTitle(conflictedName);
}

void NameConflictTitle::initTitle(const QString& conflictedName)
{
    setProperty(TITLE_FILENAME, conflictedName);
    setDisabled(false);

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
        NameConflictedStalledIssue::ConflictedNameInfo info(conflictedNames.at(index));
        QString conflictedName(info.conflictedName);

        NameConflictTitle* title(nullptr);
        if(totalUpdate)
        {
            title = nameConflictWidgets.first();
            nameConflictWidgets.removeFirst();
        }
        else
        {
            title = new NameConflictTitle(index, conflictedName, this);
            connect(title, &StalledIssueActionTitle::actionClicked, this, &NameConflict::onActionClicked);

            ui->nameConflictsLayout->addWidget(title);
        }

        if(conflictData.data->isCloud())
        {
            CloudStalledIssueDataPtr cloudData(conflictData.data->convert<CloudStalledIssueData>());
            title->updateUser(cloudData->getUserFirstName());
        }


        info.itemAttributes->requestModifiedTime(this, [title](const QDateTime& time)
        {
            title->updateLastTimeModified(time);
        });

#ifndef Q_OS_LINUX
        info.itemAttributes->requestCreatedTime(this,[this](const QDateTime& time)
        {
            title->updateCreatedTime(time);
        });
#endif

        info.itemAttributes->requestSize(this,[title](qint64 size)
        {
            title->updateSize(Utilities::getSizeString(size));
        });

        mData.data->checkTrailingSpaces(conflictedName);
        title->setTitle(conflictedName);
        title->setPath(info.conflictedPath);
        title->showIcon();

        if(title &&
                (!mData.data
                 || (mData.conflictedNames.size() > index
                 && (conflictData.conflictedNames.at(index).isSolved() != mData.conflictedNames.at(index).isSolved()))))
        {
            bool isSolved(info.isSolved());
            title->setDisabled(isSolved);
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
        setDisabled();
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

void NameConflict::setDelegate(QPointer<StalledIssueBaseDelegateWidget> newDelegate)
{
    mDelegate = newDelegate;
}

void NameConflict::setDisabled()
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

        if(actionId == RENAME_ID)
        {
            auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
            RenameNodeDialog* renameDialog(nullptr);

            if(mData.isCloud)
            {
                renameDialog = new RenameRemoteNodeDialog(info.filePath(), dialog->getDialog());
            }
            else
            {
                renameDialog = new RenameLocalNodeDialog(info.filePath(), dialog->getDialog());
            }

            renameDialog->init();

            DialogOpener::showDialog<RenameNodeDialog>(renameDialog,
                                                       [this, titleFileName, chooseTitle, renameDialog](){

                if(renameDialog->result() == QDialog::Accepted)
                {
                    auto newName = renameDialog->getName();

                    bool areAllSolved(false);

                    if(mData.isCloud)
                    {
                        areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRename(titleFileName
                                                                                                              , newName, chooseTitle->getIndex(), mDelegate->getCurrentIndex());
                    }
                    else
                    {
                        areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRename(titleFileName
                                                                                                              , newName,chooseTitle->getIndex(), mDelegate->getCurrentIndex());
                    }

                    // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
                    MegaSyncApp->getMegaApi()->clearStalledPath(mData.data->original.get());

                    if(areAllSolved)
                    {
                        emit allSolved();
                    }

                    chooseTitle->setDisabled(true);

                    //Now, close the editor because the action has been finished
                    if(mDelegate)
                    {
                        emit refreshUi();
                    }
                }
            });
        }
        else if(actionId == REMOVE_ID)
        {
            QPointer<NameConflict> currentWidget = QPointer<NameConflict>(this);
            auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

            auto isFile(false);
            QString fileName;

            if(mData.isCloud)
            {
                std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(mData.data->getFilePath().toStdString().c_str()));
                isFile = node->isFile();
                fileName = MegaNodeNames::getNodeName(node.get());
            }
            else
            {
                isFile = info.isFile();
                fileName = info.fileName();
            }

            auto warning = new QMessageBox(QMessageBox::Warning,QString::fromUtf8("MEGAsync"),
                                           tr("Are you sure you want to remove the %1 %2 %3?")
                                           .arg(mData.isCloud ? tr("remote") : tr("local"))
                                           .arg(isFile ? tr("file") : tr("folder"))
                                           .arg(fileName),
                                           QMessageBox::Yes | QMessageBox::No, dialog->getDialog());
            warning->setDefaultButton(QMessageBox::No);
            auto result = warning->exec();

            if (currentWidget && result == QMessageBox::Yes)
            {
                bool areAllSolved(false);

                if(mData.isCloud)
                {
                    areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRemove(titleFileName,chooseTitle->getIndex(), mDelegate->getCurrentIndex());
                    mUtilities.removeRemoteFile(info.filePath());
                }
                else
                {
                    areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRemove(titleFileName,chooseTitle->getIndex(), mDelegate->getCurrentIndex());
                    mUtilities.removeLocalFile(QDir::toNativeSeparators(info.filePath()));
                }

                // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
                MegaSyncApp->getMegaApi()->clearStalledPath(mData.data->original.get());

                if(areAllSolved)
                {
                    emit allSolved();
                }

                //Now, close the editor because the action has been finished
                if(mDelegate)
                {
                    emit refreshUi();
                }
            }
        }

    }
}
