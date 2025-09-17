#include "LocalAndRemoteDifferentWidget.h"

#include "LocalOrRemoteUserMustChooseStalledIssue.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "Preferences.h"
#include "StalledIssueChooseWidget.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesModel.h"
#include "TextDecorator.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QMessageBox>

const QList<mega::MegaSyncStall::SyncStallReason> ReasonsToCheck
    = QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                                                                                                           << mega::MegaSyncStall::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose;


LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> originalStall, QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    originalStall(originalStall),
    ui(new Ui::LocalAndRemoteDifferentWidget),
    mFailedItem(nullptr)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);
    connect(ui->keepBothOption, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onKeepBothButtonClicked);
    connect(ui->keepLastModifiedOption, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onKeepLastModifiedTimeButtonClicked);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->chooseLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::BODY_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    const auto issue = getData().convert<LocalOrRemoteUserMustChooseStalledIssue>();
    const auto isFailed(issue->isFailed());
    const auto chosenSide(isFailed ? LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE : issue->getChosenSide());

    if(issue->consultLocalData())
    {
        ui->chooseLocalCopy->updateUi(issue->consultLocalData(), chosenSide);

        ui->chooseLocalCopy->show();
    }
    else
    {
        ui->chooseLocalCopy->hide();
    }

    if(issue->consultCloudData())
    {
        ui->chooseRemoteCopy->updateUi(issue->consultCloudData(), chosenSide);

        ui->chooseRemoteCopy->show();
    }
    else
    {
        ui->chooseRemoteCopy->hide();
    }

    if(issue->getSyncType() != mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        GenericChooseWidget::GenericInfo bothInfo;
        bothInfo.buttonText = tr("Choose both");
        QString bothInfoTitle = tr("[B]Keep both[/B]");
        StalledIssuesBoldTextDecorator::boldTextDecorator.process(bothInfoTitle);
        bothInfo.title = bothInfoTitle;
        bothInfo.icon = Utilities::getPixmapName(QLatin1String("keep-both"),
                                                 Utilities::AttributeType::SMALL |
                                                     Utilities::AttributeType::THIN |
                                                     Utilities::AttributeType::OUTLINE);
        bothInfo.solvedText = ui->keepBothOption->chosenString();
        ui->keepBothOption->setInfo(bothInfo);

        GenericChooseWidget::GenericInfo lastModifiedInfo;
        lastModifiedInfo.buttonText = tr("Choose");
        QString lastModifiedInfoTitle;
        if(issue->lastModifiedSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL)
        {
            lastModifiedInfoTitle = tr("[B]Keep last modified[/B] (local)");
        }
        else
        {
            lastModifiedInfoTitle = tr("[B]Keep last modified[/B] (remote)");
        }

        StalledIssuesBoldTextDecorator::boldTextDecorator.process(lastModifiedInfoTitle);
        lastModifiedInfo.title = lastModifiedInfoTitle;
        lastModifiedInfo.icon = Utilities::getPixmapName(QLatin1String("clock"),
                                                         Utilities::AttributeType::SMALL |
                                                             Utilities::AttributeType::THIN |
                                                             Utilities::AttributeType::OUTLINE);
        lastModifiedInfo.solvedText = ui->keepLastModifiedOption->chosenString();
        ui->keepLastModifiedOption->setInfo(lastModifiedInfo);
    }
    else
    {
        ui->chooseRemoteCopy->setActionButtonVisibility(false);
        ui->keepBothOption->hide();
        ui->keepLastModifiedOption->hide();
    }

    if (issue->isSolved())
    {
        unSetFailedChooseWidget();

        ui->keepBothOption->setSolved(true, issue->getChosenSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::BOTH);
        ui->keepLastModifiedOption->hide();

        if (issue->isPotentiallySolved())
        {
            ui->chooseLocalCopy->setActionButtonVisibility(false);
            ui->chooseRemoteCopy->setActionButtonVisibility(false);
        }

    }
    else if(issue->isFailed())
    {
        ui->keepBothOption->setSolved(false, false);
        ui->keepLastModifiedOption->show();
        ui->chooseLocalCopy->setActionButtonVisibility(true);
        ui->chooseRemoteCopy->setActionButtonVisibility(true);

        unSetFailedChooseWidget();

        switch(issue->getChosenSide())
        {
            case LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::REMOTE:
            {
                auto errorStr = issue->consultLocalData()->isFile() ? tr("Unable to remove the local file") : tr("Unable to remove the local folder");
                ui->chooseRemoteCopy->setFailed(true, errorStr);
                mFailedItem = ui->chooseRemoteCopy;
                break;
            }
            case LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL:
            {
                auto errorStr = issue->consultLocalData()->isFile() ? tr("Unable to remove the file stored in MEGA")
                                                                    : tr("Unable to remove the folder stored in MEGA");
                ui->chooseLocalCopy->setFailed(true, errorStr);
                mFailedItem = ui->chooseLocalCopy;
                break;
            }
            case LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::BOTH:
            {
                auto errorStr = issue->consultLocalData()->isFile() ? tr("Unable to update both local and MEGA files")
                                                                    : tr("Unable to update both local and MEGA folders");
                ui->keepBothOption->setFailed(true, errorStr);
                mFailedItem = ui->keepBothOption;
                break;
            }
            default:
            {
                break;
            }
        }

    }

    updateSizeHint();
    ui->retranslateUi(this);
}

QString LocalAndRemoteDifferentWidget::keepLocalSideString(const KeepSideInfo& info)
{
    if(info.numberOfIssues > 1)
    {
        if(info.isFile)
        {
            return tr("Keep the [B]local files[/B]?");
        }
        else
        {
            return tr("Keep the [B]local folders[/B]?");
        }
    }
    else
    {
        if(info.isFile)
        {
            return tr("Are you sure you want to keep the [B]local file[/B] %1?").arg(info.itemName);
        }
        else
        {
            return tr("Are you sure you want to keep the [B]local folder[/B] %1?").arg(info.itemName);
        }
    }
}

QString LocalAndRemoteDifferentWidget::keepRemoteSideString(const KeepSideInfo& info)
{
    if(info.numberOfIssues > 1)
    {
        if(info.isFile)
        {
            return tr("Keep the [B]remote files[/B]?");
        }
        else
        {
            return tr("Keep the [B]remote folders[/B]?");
        }
    }
    else
    {
        if(info.isFile)
        {
            return tr("Are you sure you want to keep the [B]remote file[/B] %1?").arg(info.itemName);
        }
        else
        {
            return tr("Are you sure you want to keep the [B]remote folder[/B] %1?").arg(info.itemName);
        }
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{
    SelectionInfo info;
    if (!checkSelection(ReasonsToCheck, info))
    {
        return;
    }

    auto node(getNode());

    if(!node)
    {
        return;
    }

    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());

    KeepSideInfo stringInfo;
    stringInfo.isFile = localInfo.isFile();
    stringInfo.itemName = ui->chooseLocalCopy->data()->getFileName();
    stringInfo.numberOfIssues = info.selection.size();
    info.msgInfo.titleText = keepLocalSideString(stringInfo);

    if(node->isFile())
    {
        if (Preferences::instance()->fileVersioningDisabled())
        {
            if(info.selection.size() == 1)
            {
                info.msgInfo.descriptionText =
                    tr("The [B]local file[/B] %1 will be uploaded to MEGA and replace the "
                       "current file, which will be moved to the SyncDebris folder in your MEGA "
                       "Rubbish bin.")
                        .arg(localInfo.fileName()) +
                    QString::fromUtf8("[BR]");
            }
            else
            {
                info.msgInfo.descriptionText =
                    tr("The [B]local files[/B] will be uploaded to MEGA and replace the "
                       "current files, which will be moved to the SyncDebris folder in your MEGA "
                       "Rubbish bin.") +
                    QString::fromUtf8("[BR]");
            }
        }
        else
        {
            if (info.selection.size() > 1)
            {
                info.msgInfo.descriptionText =
                    (tr("The [B]local files[/B] will be uploaded to MEGA and added as a version to "
                        "the "
                        "remote files.\nPlease wait for the upload to complete.")
                         .arg(localInfo.fileName())) +
                    QString::fromUtf8("[BR]");
            }
            else
            {
                info.msgInfo.descriptionText =
                    (tr("The [B]local file[/B] %1 will be uploaded to MEGA and added as a version "
                        "to "
                        "the remote file.\nPlease wait for the upload to complete.")
                         .arg(localInfo.fileName())) +
                    QString::fromUtf8("[BR]");
            }
        }
    }
    else
    {
        info.msgInfo.descriptionText =
            tr("The [B]remote folder[/B] %1 will be moved to MEGA Rubbish Bin.[BR]You will be able "
               "to retrieve the folder from there.[/BR]")
                .arg(localInfo.fileName());
        if(info.selection.size() > 1)
        {
            info.msgInfo.descriptionText =
                tr("The [B]remote folders[/B] will be moved to MEGA Rubbish Bin.[BR]You will be "
                   "able to retrieve the folders from there.[/BR]");
        }
    }

    if(MegaSyncApp->getTransfersModel()->areAllPaused())
    {
        info.msgInfo.descriptionText.append(
            QString::fromUtf8("[BR]") +
            tr("[B]Please, resume your transfers to fix the issue[/B]", "", info.selection.size()) +
            QString::fromUtf8("[BR]"));
    }

    info.msgInfo.finishFunc = [info](QPointer<MessageDialogResult> msgBox)
    {
        if (msgBox->result() == QMessageBox::Ok)
        {
            if (msgBox->isChecked())
            {
                MegaSyncApp->getStalledIssuesModel()->chooseSideManually(false, info.similarSelection);
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->chooseSideManually(false, info.selection);
            }
        }
    };

    MessageDialogOpener::warning(info.msgInfo);
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    SelectionInfo info;
    if (!checkSelection(ReasonsToCheck, info))
    {
        return;
    }
    
    auto node(getNode());

    if(!node)
    {
        return;
    }

    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());
    if(node)
    {
        KeepSideInfo stringInfo;
        stringInfo.isFile = node->isFile();
        stringInfo.itemName = ui->chooseRemoteCopy->data()->getFileName();
        stringInfo.numberOfIssues = info.selection.size();
        info.msgInfo.titleText = keepRemoteSideString(stringInfo);
    }
    else
    {
        info.msgInfo.titleText = tr("Are you sure you want to keep the [B]remote item[/B] %1?")
                                     .arg(ui->chooseRemoteCopy->data()->getFileName());
        if (info.selection.size() > 1)
        {
            info.msgInfo.titleText = tr("Keep the [B]remote items[/B]?");
        }
    }
    //For the moment, TYPE_TWOWAY or TYPE_UNKNOWN
    if(getData().consultData()->getSyncType() != mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        if(localInfo.isFile())
        {
            info.msgInfo.descriptionText =
                tr("The [B]local file[/B] %1 will be moved to the sync debris folder")
                    .arg(localInfo.fileName());
            if (info.selection.size() > 1)
            {
                info.msgInfo.descriptionText =
                    tr("The [B]local files[/B] will be moved to the sync debris folder");
            }
        }
        else
        {
            info.msgInfo.descriptionText =
                tr("The [B]local folder[/B] %1 will be moved to the sync debris folder")
                    .arg(localInfo.fileName());
            if (info.selection.size() > 1)
            {
                info.msgInfo.descriptionText =
                    tr("The [B]local folders[/B] will be moved to the sync debris folder");
            }
        }
    }
    else
    {
        if(localInfo.isFile())
        {
            info.msgInfo.descriptionText =
                tr("The backup will be disabled in order to protect the local file %1")
                    .arg(localInfo.fileName());
            if (info.selection.size() > 1)
            {
                info.msgInfo.descriptionText =
                    tr("The backup will be disabled in order to protect the local files");
            }
        }
        else
        {
            info.msgInfo.descriptionText =
                tr("The backup will be disabled in order to protect the local folder %1")
                    .arg(localInfo.fileName());
            if (info.selection.size() > 1)
            {
                info.msgInfo.descriptionText =
                    tr("The backup will be disabled in order to protect the local folders");
            }
        }
    }

    info.msgInfo.finishFunc = [this, info](QPointer<MessageDialogResult> msgBox)
    {
        if(getData().consultData()->getSyncType() == mega::MegaSync::SyncType::TYPE_TWOWAY)
        {
            if (msgBox->result() == QMessageBox::Ok)
            {
                if (msgBox->isChecked())
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseSideManually(true, info.similarSelection);
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseSideManually(true, info.selection);
                }
            }
        }
        else
        {
            if (msgBox->result() == QMessageBox::Ok)
            {
                if (msgBox->isChecked())
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseRemoteForBackups(info.similarSelection);
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseRemoteForBackups(info.selection);
                }
            }
        }
    };

    MessageDialogOpener::warning(info.msgInfo);
}

void LocalAndRemoteDifferentWidget::onKeepBothButtonClicked(int)
{
    SelectionInfo info;
    if (!checkSelection(ReasonsToCheck, info))
    {
        return;
    }

    auto node(getNode());

    if(!node)
    {
        return;
    }

    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());
        if(localInfo.isFile())
    {
        info.msgInfo.titleText = tr("Keep both files?");
        if(info.selection.size())
        {
            info.msgInfo.titleText = tr("Keep all files?");
        }
    }
    else
    {
        info.msgInfo.titleText = tr("Keep both folders");
        if(info.selection.size())
        {
            info.msgInfo.titleText = tr("Keep all folders");
        }
    }

    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getParentNode(node.get()));
    if(parentNode)
    {
        //auto newName = Utilities::getNonDuplicatedNodeName(node.get(), parentNode.get(), QString::fromUtf8(node->getName()), true, QStringList());

        if(node->isFile())
        {
            info.msgInfo.descriptionText =
                tr("The [B]remote file[/B] will have a suffix like (1) added",
                   "",
                   info.selection.size());
        }
        else
        {
            info.msgInfo.descriptionText =
                tr("The [B]remote folder[/B] will have a suffix like (1) added",
                   "",
                   info.selection.size());
        }

        info.msgInfo.finishFunc = [info](QPointer<MessageDialogResult> msgBox)
        {
            if (msgBox->result() == QMessageBox::Ok)
            {
                if (msgBox->isChecked())
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseBothSides(info.similarSelection);
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseBothSides(info.selection);
                }
            }
        };

        MessageDialogOpener::warning(info.msgInfo);
    }
}

void LocalAndRemoteDifferentWidget::onKeepLastModifiedTimeButtonClicked(int)
{
    auto issue = getData().convert<LocalOrRemoteUserMustChooseStalledIssue>();

    SelectionInfo info;
    if (!checkSelection(ReasonsToCheck, info))
    {
        return;
    }

    info.msgInfo.titleText = tr("Are you sure you want to choose the latest modified side?");
    if(issue->lastModifiedSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL)
    {
        info.msgInfo.descriptionText = tr("This action will choose the local side");
    }
    else
    {
        info.msgInfo.descriptionText = tr("This action will choose the remote side");
    }

    info.msgInfo.finishFunc = [info](QPointer<MessageDialogResult> msgBox)
    {
        if (msgBox->result() == QMessageBox::Ok)
        {
            if (msgBox->isChecked())
            {
                MegaSyncApp->getStalledIssuesModel()->chooseLastModifiedLocalRemoteIssues(
                    info.similarSelection);
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->chooseLastModifiedLocalRemoteIssues(
                    info.selection);
            }
        }
    };

    MessageDialogOpener::warning(info.msgInfo);
}

void LocalAndRemoteDifferentWidget::unSetFailedChooseWidget()
{
    if(mFailedItem)
    {
        mFailedItem->setFailed(false);
        mFailedItem = nullptr;
    }
}

std::unique_ptr<mega::MegaNode> LocalAndRemoteDifferentWidget::getNode()
{
    auto cloudData = ui->chooseRemoteCopy->data()->convert<CloudStalledIssueData>();

    if(!cloudData)
    {
        return nullptr;
    }

    std::unique_ptr<mega::MegaNode> node(
        MegaSyncApp->getMegaApi()->getNodeByHandle(cloudData->getPathHandle()));

    return node;
}
