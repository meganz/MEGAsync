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
#include <QPainter>

static const int RENAME_ID = 0;
static const int REMOVE_ID = 1;

const char* TITLE_FILENAME = "TITLE_FILENAME";
const char* TITLE_INDEX = "TITLE_INDEX";

//NAME DUPLICATED
void NameDuplicatedContainer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);

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
}

//NAME CONFLICT
NameConflict::NameConflict(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameConflict)
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
    auto nameData = getData(issue);

    //Fill conflict names
    auto conflictedNames = getConflictedNames(issue);

    //Reset widgets
    bool firstTime(mTitlesByIndex.isEmpty());
    bool allSolved(true);

    for(int index = conflictedNames.size()-1; index >= 0; index--)
    {
        std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info(conflictedNames.at(index));
        QString conflictedName(info->getConflictedName());

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

        if(firstTime)
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
        }

        nameData->checkTrailingSpaces(conflictedName);

        title->setInfo(info->mConflictedPath, info->mHandle);
        title->setIsCloud(isCloud());
        title->showIcon();

        if(title && info->isSolved() != title->isSolved())
        {
            bool isSolved(info->isSolved());
            title->setSolved(isSolved);
            if(isSolved)
            {
                title->hideActionButton(RENAME_ID);
                title->hideActionButton(REMOVE_ID);

                QIcon icon;
                QString titleText;

                if(info->mSolved ==  NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE)
                {
                    icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                    titleText = tr("Removed");
                }
                else if(info->mSolved ==  NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME)
                {
                    icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                    titleText = tr("Renamed to \"%1\"").arg(info->mRenameTo);
                }
                else
                {
                    icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                    titleText = tr("No action needed");
                }

                titleLayout->activate();
                title->setMessage(titleText, icon.pixmap(24,24));
            }
        }

        allSolved &= info->isSolved();

        titleLayout->activate();
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
        setDisabled();
    }

    mIssue = issue;
}

void NameConflict::initTitle(StalledIssueActionTitle* title, int index, const QString& conflictedName)
{
    title->setProperty(TITLE_FILENAME, conflictedName);
    title->setProperty(TITLE_INDEX, index);
    title->setSolved(false);

    QIcon renameIcon(QString::fromUtf8("://images/StalledIssues/rename_node_default.png"));
    QIcon removeIcon(QString::fromUtf8("://images/StalledIssues/remove_default.png"));
    title->addActionButton(renameIcon, tr("Rename"), RENAME_ID, false);
    title->addActionButton(removeIcon, QString(), REMOVE_ID, false);
}

void NameConflict::onRawInfoChecked()
{
    foreach(auto title, mTitlesByIndex)
    {
        //Fill conflict names
        auto conflictedNames = getConflictedNames(mIssue);
        std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info(conflictedNames.at(title->property(TITLE_INDEX).toInt()));
        updateTitleExtraInfo(title, info);
    }

    //To let the view know that the size may change
    mDelegateWidget->updateSizeHint();
}

void NameConflict::updateTitleExtraInfo(StalledIssueActionTitle* title, std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info)
{
    auto index = title->property(TITLE_INDEX).toInt();

    info->mItemAttributes->requestModifiedTime(title, [this, index](const QDateTime& time)
    {
        mTitlesByIndex.value(index)->updateLastTimeModified(time);
    });

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
    //User
    if(isCloud())
    {
        auto cloudAttributes(FileFolderAttributes::convert<RemoteFileFolderAttributes>(info->mItemAttributes));

        cloudAttributes->requestVersions(title,[this, index](int versions)
        {
            mTitlesByIndex.value(index)->updateVersionsCount(versions);
        });

        cloudAttributes->requestUser(title, MegaSyncApp->getMegaApi()->getMyUserHandleBinary(), [this, index](QString user, bool showAttribute)
        {
            mTitlesByIndex.value(index)->updateUser(user, showAttribute);
        });
    }

    title->updateExtraInfoLayout();
}


void NameConflict::setDelegate(QPointer<StalledIssueBaseDelegateWidget> newDelegate)
{
    mDelegateWidget = newDelegate;
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
    if(auto chooseTitle = dynamic_cast<StalledIssueActionTitle*>(sender()))
    {
        auto issueData = getData(mIssue);
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        QFileInfo info;
        auto titleFileName = chooseTitle->property(TITLE_FILENAME).toString();
        info.setFile(issueData->getNativePath(), titleFileName);
        QString filePath(info.filePath());

        auto conflictedNames(getConflictedNames(mIssue));
        auto conflictIndex(chooseTitle->property(TITLE_INDEX).toInt());

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
                renameDialog = new RenameLocalNodeDialog(filePath, dialog->getDialog());
            }

            renameDialog->init();

            DialogOpener::showDialog<RenameNodeDialog>(renameDialog,
                                                       [this, issueData, titleFileName, conflictIndex, renameDialog](){

                if(renameDialog->result() == QDialog::Accepted)
                {
                    auto newName = renameDialog->getName();

                    bool areAllSolved(false);

                    if(isCloud())
                    {
                        areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRename(newName, conflictIndex, mDelegateWidget->getCurrentIndex());
                    }
                    else
                    {
                        areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRename(newName, conflictIndex, mDelegateWidget->getCurrentIndex());
                    }

                    // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
                    MegaSyncApp->getMegaApi()->clearStalledPath(issueData->original.get());

                    if(areAllSolved)
                    {
                        emit allSolved();
                    }

                    //Now, close the editor because the action has been finished
                    if(mDelegateWidget)
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
                auto conflictedName(conflictedNames.at(conflictIndex));
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
                isFile = info.isFile();
                fileName = info.fileName();
            }

            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
            msgInfo.title = MegaSyncApp->getMEGAString();
            msgInfo.textFormat = Qt::RichText;
            msgInfo.buttons = QMessageBox::Yes | QMessageBox::Cancel;

            msgInfo.text = tr("Are you sure you want to remove the %1 %2 %3?")
                    .arg(isCloud() ? tr("remote") : tr("local"))
                    .arg(isFile ? tr("file") : tr("folder"))
                    .arg(fileName);

            if(isCloud())
            {
                if(isFile)
                {
                    msgInfo.informativeText = tr("It will be moved to MEGA Rubbish Bin along with its versions.<br>You will be able to retrieve the file and its versions from there.</br>");
                }
                else
                {
                    msgInfo.informativeText = tr("It will be moved to MEGA Rubbish Bin.<br>You will be able to retrieve the folder from there.</br>");
                }
            }

            msgInfo.finishFunc = [this, issueData, handle, filePath, titleFileName, conflictIndex](QMessageBox* msgBox)
            {
                if (msgBox->result() == QDialogButtonBox::Yes)
                {
                    bool areAllSolved(false);

                    if(isCloud())
                    {
                        areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveCloudConflictedNameByRemove(conflictIndex, mDelegateWidget->getCurrentIndex());
                        if(handle != mega::INVALID_HANDLE)
                        {
                            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
                            if(node)
                            {
                                mUtilities.removeRemoteFile(node.get());
                            }
                        }
                        else
                        {
                            mUtilities.removeRemoteFile(filePath);
                        }
                    }
                    else
                    {
                        areAllSolved = MegaSyncApp->getStalledIssuesModel()->solveLocalConflictedNameByRemove(conflictIndex, mDelegateWidget->getCurrentIndex());
                        mUtilities.removeLocalFile(QDir::toNativeSeparators(filePath));
                    }

                    // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
                    MegaSyncApp->getMegaApi()->clearStalledPath(issueData->original.get());

                    if(areAllSolved)
                    {
                        emit allSolved();
                    }

                    //Now, close the editor because the action has been finished
                    if(mDelegateWidget)
                    {
                        emit refreshUi();
                    }
                }
            };

            QMegaMessageBox::warning(msgInfo);
        }
    }
}
