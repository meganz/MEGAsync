#include "NameConflict.h"

#include "DialogOpener.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "RenameNodeDialog.h"
#include "StalledIssuesDialog.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesUtilities.h"
#include "StatsEventHandler.h"
#include "TextDecorator.h"
#include "ui_NameConflict.h"

#include <QDialogButtonBox>
#include <QPainter>

static const int RENAME_ID = 0;
static const int REMOVE_ID = 1;

const char* TITLE_FILENAME = "TITLE_FILENAME";
const char* TITLE_INDEX = "TITLE_INDEX";

//NAME DUPLICATED
void NameDuplicatedContainer::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform);

    QRect textRect(5,10, width() - 5, 20);
    QFont font = painter.font();

    font.setPointSize(Utilities::getDevicePixelRatio() < 2 ? 10 : 14);
    font.setItalic(true);
    font.setFamily(QLatin1String("Lato Semibold"));
    painter.setFont(font);
    painter.setPen(Qt::gray);
    painter.drawText(textRect, tr("Duplicated"), QTextOption(Qt::AlignLeft));

    QRect horRect1(0, 5, 10, 2);
    painter.fillRect(horRect1, Qt::gray);

    QRect LineRect(0, 5, 2, height() - 10);
    painter.fillRect(LineRect, Qt::gray);

    QRect horRect2(0, height() - 7, 10, 2);
    painter.fillRect(horRect2, Qt::gray);

    QWidget::paintEvent(event);
}

//NAME CONFLICT
NameConflict::NameConflict(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameConflict),
    mSolvedStatusAppliedToUi(false)
{
    ui->setupUi(this);

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::showRawInfoChanged, this, &NameConflict::onRawInfoChecked);
}

NameConflict::~NameConflict()
{
    delete ui;
}

void NameConflict::updateUi(std::shared_ptr<const NameConflictedStalledIssue> issue)
{
    bool duplicatedRemoved(false);

    mIssue = issue;
    auto nameData = getData();

    //Fill conflict names
    auto conflictedNamesInfo = getConflictedNamesInfo();

    //Reset widgets
    bool allSolved(true);

    for(int index = conflictedNamesInfo.size()-1; index >= 0; index--)
    {
        std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info(conflictedNamesInfo.at(index));
        QString conflictedName(getConflictedName(info));

        QWidget* parent(ui->nameConflicts);
        QVBoxLayout* titleLayout(ui->nameConflictsLayout);

        if(info->mDuplicated)
        {
            QPointer<QWidget> groupContainer = mContainerByDuplicateByGroupId[info->mDuplicatedGroupId];

            if(!groupContainer)
            {
                groupContainer = new NameDuplicatedContainer(this);
                QVBoxLayout* layout = new QVBoxLayout(groupContainer);
                mContainerByDuplicateByGroupId.insert(info->mDuplicatedGroupId, groupContainer);
                groupContainer->setLayout(layout);
                layout->setContentsMargins(5,35,0,10);

                ui->nameConflictsLayout->addWidget(groupContainer);
                ui->nameConflictsLayout->activate();
            }

            parent = groupContainer;
            titleLayout = dynamic_cast<QVBoxLayout*>(groupContainer->layout());
        }

        StalledIssueActionTitle* title(nullptr);

        if(!mTitlesByIndex.contains(index))
        {
            title = new StalledIssueActionTitle(parent);
            initTitle(title, index, conflictedName);

            connect(title, &StalledIssueActionTitle::actionClicked, this, &NameConflict::onActionClicked);
            titleLayout->addWidget(title);
            mTitlesByIndex.insert(index, title);
        }
        else
        {
            title = mTitlesByIndex.value(index);

            if(!info->mDuplicated && title->parent() != ui->nameConflicts)
            {
                QPointer<QWidget> groupContainer = mContainerByDuplicateByGroupId[info->mDuplicatedGroupId];

                if(groupContainer)
                {
                    groupContainer->layout()->removeWidget(title);
                    if(groupContainer->layout()->count() < 2)
                    {
                        if(groupContainer->layout()->count() == 1)
                        {
                            auto lastTitle = groupContainer->layout()->takeAt(0)->widget();
                            ui->nameConflictsLayout->addWidget(lastTitle);
                        }

                        mContainerByDuplicateByGroupId.remove(info->mDuplicatedGroupId);
                        groupContainer->deleteLater();
                        duplicatedRemoved = true;
                    }
                }

                ui->nameConflictsLayout->addWidget(title);
            }
        }

        nameData->checkTrailingSpaces(conflictedName);

        title->setIsCloud(isCloud());
        title->setInfo(info->mConflictedPath, info->mHandle);
        title->setIsFile(info->mIsFile);
        title->showIcon();
        title->setFailed(false, QString());

        if(info->isSolved() && !mSolvedStatusAppliedToUi)
        {
            title->setDisable(true);
            title->setActionButtonVisibility(RENAME_ID, false);
            title->setActionButtonVisibility(REMOVE_ID, false);

            QIcon icon;
            QString titleText;

            if (info->getSolvedType() == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE)
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                titleText = tr("Removed");
            }
            else if (info->getSolvedType() ==
                     NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME)
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                titleText = tr("Renamed to \"%1\"").arg(info->mRenameTo);
            }
            else if (info->getSolvedType() ==
                     NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::MERGED)
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                titleText = tr("Merged");
            }
            else if (info->getSolvedType() ==
                     NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::CHANGED_EXTERNALLY)
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                titleText = tr("Modified externally");
            }
            else
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                titleText = tr("No action needed");
            }

            titleLayout->activate();
            title->setMessage(titleText, icon.pixmap(16, 16));
        }
        else if(info->isFailed())
        {
            titleLayout->activate();
            title->setFailed(true, info->mError);
        }

        if(!issue->isSolved())
        {
            initActionButtons(title);
        }

        allSolved &= info->isSolved();

        titleLayout->activate();
        parent->updateGeometry();
        title->setTitle(conflictedName);
        updateTitleExtraInfo(title, info);

    }

    if(nameData)
    {
        ui->path->show();
        ui->path->updateUi(nameData);
    }
    else
    {
        ui->path->hide();
    }

    if(allSolved)
    {
        mSolvedStatusAppliedToUi = true;
    }

    ui->nameConflicts->layout()->activate();
    ui->nameConflicts->updateGeometry();
    layout()->activate();
    updateGeometry();

    if(duplicatedRemoved)
    {
        mDelegateWidget->updateSizeHint();
    }
}

void NameConflict::initTitle(StalledIssueActionTitle* title, int index, const QString& conflictedName)
{
    title->setProperty(TITLE_FILENAME, conflictedName);
    title->setProperty(TITLE_INDEX, index);
    title->setDisable(false);
}

void NameConflict::initActionButtons(StalledIssueActionTitle* title)
{
    QIcon renameIcon(QString::fromUtf8("://images/StalledIssues/rename_node_default.png"));
    QIcon removeIcon(QString::fromUtf8("://images/StalledIssues/remove_default.png"));
    title->addActionButton(renameIcon, tr("Rename"), RENAME_ID, false, QLatin1String("secondary"));
    title->addActionButton(removeIcon, QString(), REMOVE_ID, false, QLatin1String("icon"));
}

void NameConflict::onRawInfoChecked()
{
    const auto conflictedNames = getConflictedNamesInfo();
    foreach(auto title, mTitlesByIndex)
    {
        //Fill conflict names
        std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info(conflictedNames.at(title->property(TITLE_INDEX).toInt()));
        updateTitleExtraInfo(title, info);
    }

    //To let the view know that the size may change
    mDelegateWidget->updateSizeHint();
}

void NameConflict::updateTitleExtraInfo(StalledIssueActionTitle* title, std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info)
{
    auto index = title->property(TITLE_INDEX).toInt();

    if (!isCloud() || info->mIsFile)
    {
        info->mItemAttributes->requestModifiedTime(title, [this, index](const QDateTime& time) {
            mTitlesByIndex.value(index)->updateLastTimeModified(time);
        });
    }

    if(isCloud())
    {
        info->mItemAttributes->requestCreatedTime(title, [this, index](const QDateTime& time)
        {
            mTitlesByIndex.value(index)->updateCreatedTime(time);
        });
    }
    else
    {
#ifndef Q_OS_LINUX
        info->mItemAttributes->requestCreatedTime(title, [this, index](const QDateTime& time)
        {
            mTitlesByIndex.value(index)->updateCreatedTime(time);
        });
#endif
    }

    info->mItemAttributes->requestSize(title,[this, index](qint64 size)
    {
        mTitlesByIndex.value(index)->updateSize(size);
    });

    if(MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible())
    {
        info->mItemAttributes->requestCRC(title, [this, index](const QString& fp)
        {
            mTitlesByIndex.value(index)->updateCRC(fp);
        });
    }
    else
    {
        title->updateCRC(QString());
    }

    //These items go in order
    //Modified time -- created time -- size
    //Versions count -- User
    if(isCloud())
    {
        auto cloudAttributes(FileFolderAttributes::convert<RemoteFileFolderAttributes>(info->mItemAttributes));

        cloudAttributes->requestVersions(title,[this, index](int versions)
        {
            if(mTitlesByIndex.value(index)->updateVersionsCount(versions))
            {
                mDelegateWidget->updateSizeHint();
            }
        });

        cloudAttributes->requestUser(title, [this, cloudAttributes, index](QString user)
        {
            if(mTitlesByIndex.value(index)->updateUser(user, !cloudAttributes->isCurrentUser()))
            {
                mDelegateWidget->updateSizeHint();
            }
        });
    }

    title->updateExtraInfoLayout();
}

void NameConflict::setDelegate(QPointer<StalledIssueBaseDelegateWidget> newDelegate)
{
    mDelegateWidget = newDelegate;
}

QString NameConflict::getConflictedName(std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info) const
{
    return info->getConflictedName();
}

void NameConflict::onActionClicked(int actionId)
{
    if(auto chooseTitle = dynamic_cast<StalledIssueActionTitle*>(sender()))
    {
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        if(MegaSyncApp->getStalledIssuesModel()->checkForExternalChanges(mDelegateWidget->getCurrentIndex()))
        {
            mDelegateWidget->updateSizeHint();

            return;
        }

        auto issueData = getData();

        QFileInfo info;
        auto titleFileName = chooseTitle->property(TITLE_FILENAME).toString();
        info.setFile(issueData->getNativePath(), titleFileName);
        QString filePath(info.filePath());

        auto conflictedNames(getConflictedNamesInfo());
        auto conflictIndex(chooseTitle->property(TITLE_INDEX).toInt());

        if(conflictedNames.size() <= conflictIndex)
        {
            return;
        }

        auto conflictedName(conflictedNames.at(conflictIndex));
        if(!conflictedName)
        {
            return;
        }

        if(actionId == RENAME_ID)
        {
            RenameNodeDialog* renameDialog(nullptr);

            if(isCloud())
            {
                std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNames.at(conflictIndex)->mHandle));
                if(node)
                {
                    renameDialog = new RenameRemoteNodeDialog(std::move(node), dialog->getDialog());
                }
                else
                {
                    renameDialog = new RenameRemoteNodeDialog(filePath, dialog->getDialog());
                }
            }
            else
            {
                if(info.exists())
                {
                    renameDialog = new RenameLocalNodeDialog(filePath, dialog->getDialog());
                }
                else
                {
                    MessageDialogInfo msgInfo;
                    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
                    msgInfo.textFormat = Qt::RichText;
                    msgInfo.descriptionText =
                        tr("%1 no longer exists.\nPlease refresh the view").arg(info.fileName());
                    MessageDialogOpener::warning(msgInfo);
                    return;
                }
            }

            renameDialog->init();

            DialogOpener::showDialog<RenameNodeDialog>(renameDialog,
                [=]()
                {
                    if (renameDialog->result() == QDialog::Accepted)
                    {
                        auto newName = renameDialog->getName();
                        auto originalName = renameDialog->getOriginalName();

                        bool areAllSolved(false);

                        if (isCloud())
                        {
                            areAllSolved = MegaSyncApp->getStalledIssuesModel()
                                               ->solveCloudConflictedNameByRename(newName, originalName,
                                                   conflictIndex,
                                                   mDelegateWidget->getCurrentIndex());
                        }
                        else
                        {
                            areAllSolved = MegaSyncApp->getStalledIssuesModel()
                                               ->solveLocalConflictedNameByRename(newName, originalName,
                                                   conflictIndex,
                                                   mDelegateWidget->getCurrentIndex());
                        }



                        checkIfAreAllSolved(areAllSolved);

                        //Now, close the editor because the action has been finished
                        if (mDelegateWidget)
                        {
                            emit refreshUi();

                        }
                    }
                });
        }
        else if(actionId == REMOVE_ID)
        {
            auto isFile(false);
            QString fileName;
            mega::MegaHandle handle(mega::INVALID_HANDLE);

            if(isCloud())
            {
                if(conflictedName)
                {
                    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedName->mHandle));
                    if(node)
                    {
                        handle = node->getHandle();
                        isFile = node->isFile();
                        fileName = MegaNodeNames::getNodeName(node.get());
                    }
                }
            }
            else
            {
                if(info.exists())
                {
                    isFile = info.isFile();
                    fileName = info.fileName();
                }
                else
                {
                    MessageDialogInfo msgInfo;
                    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
                    msgInfo.textFormat = Qt::RichText;
                    msgInfo.descriptionText =
                        tr("%1 no longer exists.\nPlease refresh the view").arg(info.fileName());
                    MessageDialogOpener::warning(msgInfo);
                    return;
                }
            }

            MessageDialogInfo msgInfo;
            msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
            msgInfo.textFormat = Qt::RichText;
            msgInfo.buttons = QMessageBox::Yes | QMessageBox::Cancel;
            msgInfo.defaultButton = QMessageBox::Yes;

            if(isCloud())
            {
                if(isFile)
                {
                    msgInfo.titleText =
                        tr("Are you sure you want to remove the remote file %1?").arg(fileName);
                    msgInfo.descriptionText =
                        tr("It will be moved to the SyncDebris folder on the MEGA Rubbish Bin "
                           "along with its versions.[BR]You will be able to retrieve the file and "
                           "its versions from there.[/BR]");
                }
                else
                {
                    msgInfo.titleText =
                        tr("Are you sure you want to remove the remote folder %1?").arg(fileName);
                    msgInfo.descriptionText =
                        tr("It will be moved to the SyncDebris folder on the MEGA Rubbish "
                           "Bin.[BR]You will be able to retrieve the folder from there.[/BR]");
                }
            }
            else
            {
                if(isFile)
                {
                    msgInfo.titleText =
                        tr("Are you sure you want to remove the local file %1?").arg(fileName);
                    msgInfo.descriptionText =
                        tr("It will be moved to the sync rubbish folder.[BR]You will be able to "
                           "retrieve the file from there.[/BR]");
                }
                else
                {
                    msgInfo.titleText =
                        tr("Are you sure you want to remove the local folder %1?").arg(fileName);
                    msgInfo.descriptionText =
                        tr("It will be moved to the sync rubbish folder.[BR]You will be able to "
                           "retrieve the folder from there.[/BR]");
                }
            }

            msgInfo.finishFunc = [=](QPointer<MessageDialogResult> msgBox)
            {
                if (msgBox->result() == QMessageBox::Yes)
                {
                    bool areAllSolved(false);

                    if(isCloud())
                    {
                        std::shared_ptr<mega::MegaError> error;
                        if(handle != mega::INVALID_HANDLE)
                        {
                            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
                            if(node)
                            {
                                error = Utilities::removeSyncRemoteFile(node.get());
                            }
                        }
                        else
                        {
                            error = Utilities::removeSyncRemoteFile(filePath);
                        }

                        if(error)
                        {
                            QString errorStr = isFile ?  StalledIssuesStrings::RemoveRemoteFailedFile(error.get()) : StalledIssuesStrings::RemoveRemoteFailedFolder(error.get());
                            MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameFailed(conflictIndex, mDelegateWidget->getCurrentIndex(), errorStr);
                            NameConflictedStalledIssue::showRemoteRenameHasFailedMessageBox((*error.get()), isFile);
                        }
                        else
                        {
                            areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRemove(conflictIndex, mDelegateWidget->getCurrentIndex());
                        }
                    }
                    else
                    {
                        auto syncId = mIssue->syncIds().isEmpty() ? mega::INVALID_HANDLE : mIssue->firstSyncId();
                        auto failed = !Utilities::removeLocalFile(QDir::toNativeSeparators(filePath), syncId);

                        if(failed)
                        {
                            QString errorStr = isFile ?  StalledIssuesStrings::RemoveLocalFailedFile() : StalledIssuesStrings::RemoveLocalFailedFolder();

                            MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameFailed(conflictIndex, mDelegateWidget->getCurrentIndex(), errorStr);
                            NameConflictedStalledIssue::showLocalRenameHasFailedMessageBox(isFile);
                        }
                        else
                        {
                            areAllSolved =
                                MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRemove(
                                    conflictIndex, mDelegateWidget->getCurrentIndex());
                        }
                    }

                    checkIfAreAllSolved(areAllSolved);

                    //Now, close the editor because the action has been finished
                    if(mDelegateWidget)
                    {
                        emit refreshUi();
                    }
                }
            };
            MessageDialogOpener::warning(msgInfo);
        }
    }
}

void NameConflict::checkIfAreAllSolved(bool areAllSolved)
{
    if(areAllSolved)
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_MANUALLY);
        emit allSolved();
    }
}
