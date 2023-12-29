#include "LocalAndRemoteDifferentWidget.h"
#include <QDialogButtonBox>
#include "ui_LocalAndRemoteDifferentWidget.h"
#include <QCheckBox>

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <PlatformStrings.h>
#include <QMegaMessageBox.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>

#include "mega/types.h"

#include <QMessageBox>
#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> originalStall, QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    originalStall(originalStall),
    ui(new Ui::LocalAndRemoteDifferentWidget)
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
    auto issue = getData().convert<LocalOrRemoteUserMustChooseStalledIssue>();

    if(issue->consultLocalData())
    {
        ui->chooseLocalCopy->updateUi(issue->consultLocalData(), issue->getChosenSide());

        ui->chooseLocalCopy->show();
    }
    else
    {
        ui->chooseLocalCopy->hide();
    }

    if(issue->consultCloudData())
    {
        ui->chooseRemoteCopy->updateUi(issue->consultCloudData(), issue->getChosenSide());

        ui->chooseRemoteCopy->show();
    }
    else
    {
        ui->chooseRemoteCopy->hide();
    }

    GenericChooseWidget::GenericInfo bothInfo;
    bothInfo.buttonText = tr("Choose both");
    bothInfo.title = tr("<b>Keep both</b>");
    bothInfo.icon = QLatin1String(":/images/copy.png");
    bothInfo.solvedText = tr("Chosen");
    ui->keepBothOption->setInfo(bothInfo);

    GenericChooseWidget::GenericInfo lastModifiedInfo;
    lastModifiedInfo.buttonText = tr("Choose");
    lastModifiedInfo.title =
        tr("<b>Keep last modified</b> (%1)")
            .arg(issue->lastModifiedSide() ==
                         LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::Local
                     ? tr("Local")
                     : tr("Remote"));
    lastModifiedInfo.icon = QLatin1String(":/images/clock_ico.png");
    lastModifiedInfo.solvedText = tr("Chosen");
    ui->keepLastModifiedOption->setInfo(lastModifiedInfo);

    if (issue->isSolved())
    {
        ui->keepBothOption->setChosen(false);
        ui->keepLastModifiedOption->hide();

        if(issue->isPotentiallySolved())
        {
            ui->chooseLocalCopy->hideActionButton();
            ui->chooseRemoteCopy->hideActionButton();
        }
        else
        {
            if(issue->getChosenSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::Both)
            {
                ui->keepBothOption->setChosen(true);
            }
        }

        updateSizeHint();
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{
    SelectionInfo info;
    if (!checkSelection(info))
    {
        return;
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());
    if(localInfo.isFile())
    {
        info.msgInfo.text = tr("Are you sure you want to keep the <b>local file</b> %1?", "", info.selection.size()).arg(ui->chooseLocalCopy->data()->getFileName());
    }
    else
    {
        info.msgInfo.text = tr("Are you sure you want to keep the <b>local folder</b> %1?", "", info.selection.size()).arg(ui->chooseLocalCopy->data()->getFileName());
    }

    if(node->isFile())
    {
        info.msgInfo.informativeText = tr("The <b>local file</b> %1 will be uploaded to MEGA and added as a version to the remote file.\nPlease wait for the upload to complete.</br>", "", info.selection.size()).arg(localInfo.fileName());
    }
    else
    {
        info.msgInfo.informativeText = tr("The <b>remote folder</b> %1 will be moved to MEGA Rubbish Bin.<br>You will be able to retrieve the folder from there.</br>", "", info.selection.size()).arg(localInfo.fileName());
    }

    if(MegaSyncApp->getTransfersModel()->areAllPaused())
    {
        info.msgInfo.informativeText.append(tr("<br><b>Please, resume your transfers to fix the issue</b></br>", "", info.selection.size()));
    }

    info.msgInfo.finishFunc = [info](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            if(msgBox->checkBox() && msgBox->checkBox()->isChecked())
            {
                MegaSyncApp->getStalledIssuesModel()->chooseSideManually(false, info.similarSelection);
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->chooseSideManually(false, info.selection);
            }
        }
    };

    QMegaMessageBox::warning(info.msgInfo);
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    SelectionInfo info;
    if (!checkSelection(info))
    {
        return;
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());
    if(node)
    {
        if(node->isFile())
        {
            info.msgInfo.text = tr("Are you sure you want to keep the <b>remote file</b> %1?", "", info.selection.size()).arg(ui->chooseRemoteCopy->data()->getFileName());
        }
        else
        {
            info.msgInfo.text = tr("Are you sure you want to keep the <b>remote folder</b> %1?", "", info.selection.size()).arg(ui->chooseRemoteCopy->data()->getFileName());
        }
    }
    else
    {
        info.msgInfo.text = tr("Are you sure you want to keep the <b>remote item</b> %1?", "", info.selection.size()).arg(ui->chooseRemoteCopy->data()->getFileName());
    }

    //For the moment, TYPE_TWOWAY or TYPE_UNKNOWN
    if(getData().consultData()->getSyncType() != mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        if(localInfo.isFile())
        {
            info.msgInfo.informativeText = tr("The <b>local file</b> %1 will be moved to the sync debris folder").arg(localInfo.fileName());
        }
        else
        {
            info.msgInfo.informativeText = tr("The <b>local folder</b> %1 will be moved to the sync debris folder").arg(localInfo.fileName());
        }
    }
    else
    {
        if(localInfo.isFile())
        {
            info.msgInfo.informativeText = tr("The backup will be disabled in order to protect the local file %1", "", info.selection.size()).arg(localInfo.fileName());
        }
        else
        {
            info.msgInfo.informativeText = tr("The backup will be disabled in order to protect the local folder %1", "", info.selection.size()).arg(localInfo.fileName());
        }
    }

    info.msgInfo.finishFunc = [this, info](QMessageBox* msgBox)
    {
        if(getData().consultData()->getSyncType() == mega::MegaSync::SyncType::TYPE_TWOWAY)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                if(msgBox->checkBox() && msgBox->checkBox()->isChecked())
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
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                if(msgBox->checkBox() && msgBox->checkBox()->isChecked())
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

    QMegaMessageBox::warning(info.msgInfo);
}

void LocalAndRemoteDifferentWidget::onKeepBothButtonClicked(int)
{
    SelectionInfo info;
    if (!checkSelection(info))
    {
        return;
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());
    if(localInfo.isFile())
    {
        info.msgInfo.text = tr(
            "Are you sure you want to keep both file?", "", info.selection.size());
    }
    else
    {
        info.msgInfo.text = tr(
            "Are you sure you want to keep both folder %1?", "", info.selection.size());
    }

    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getParentNode(node.get()));
    if(parentNode)
    {
        auto newName = Utilities::getNonDuplicatedNodeName(node.get(), parentNode.get(), QString::fromUtf8(node->getName()), true, QStringList());

        if(node->isFile())
        {
            info.msgInfo.informativeText = tr("The <b>remote file</b> will be renamed to %1", "", info.selection.size()).arg(newName);
        }
        else
        {
            info.msgInfo.informativeText = tr("The <b>remote file</b> will be renamed to %1", "", info.selection.size()).arg(newName);
        }

        info.msgInfo.finishFunc = [info](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                if(msgBox->checkBox() && msgBox->checkBox()->isChecked())
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseBothSides(info.similarSelection);
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->chooseBothSides(info.selection);
                }
            }
        };

        QMegaMessageBox::warning(info.msgInfo);
    }
}

void LocalAndRemoteDifferentWidget::onKeepLastModifiedTimeButtonClicked(int)
{
    auto issue = getData().convert<LocalOrRemoteUserMustChooseStalledIssue>();

    SelectionInfo info;
    if (!checkSelection(info))
    {
        return;
    }

    info.msgInfo.text = tr("Are you sure you want to choose the latest modified side?");
    if(issue->lastModifiedSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::Local)
    {
        info.msgInfo.informativeText = tr("This action will choose the local side");
    }
    else
    {
        info.msgInfo.informativeText = tr("This action will choose the remote side");
    }

    info.msgInfo.finishFunc = [info](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            if(msgBox->checkBox() && msgBox->checkBox()->isChecked())
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveLocalRemoteIssues(info.similarSelection);
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveLocalRemoteIssues(info.selection);
            }
        }
    };

    QMegaMessageBox::warning(info.msgInfo);
}

bool LocalAndRemoteDifferentWidget::checkIssue(QDialog *dialog)
{
    if(MegaSyncApp->getStalledIssuesModel()->checkForExternalChanges(getCurrentIndex()))
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok;
        QMap<QMessageBox::StandardButton, QString> buttonsText;
        buttonsText.insert(QMessageBox::Ok, tr("Refresh"));
        msgInfo.buttonsText = buttonsText;
        msgInfo.text = tr("The issue may have been solved externally.\nPlease, refresh the list.");
        msgInfo.finishFunc = [this](QPointer<QMessageBox>){
            MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
        };

        QMegaMessageBox::warning(msgInfo);


        ui->chooseLocalCopy->hideActionButton();
        ui->chooseRemoteCopy->hideActionButton();

        updateSizeHint();

        return true;
    }

    return false;
}

bool LocalAndRemoteDifferentWidget::checkSelection(SelectionInfo& info)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    if(checkIssue(dialog ? dialog->getDialog() : nullptr))
    {
        return false;
    }

    info.msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    info.msgInfo.title = MegaSyncApp->getMEGAString();
    info.msgInfo.textFormat = Qt::RichText;
    info.msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;

    info.msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));
    textsByButton.insert(QMessageBox::Ok, tr("Apply"));

    auto reasons(QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                                                               << mega::MegaSyncStall::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose);
    info.selection = dialog->getDialog()->getSelection(reasons);
    info.similarSelection = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);
    if(info.similarSelection.size() != info.selection.size())
    {
        auto checkBox = new QCheckBox(tr("Apply to all"));
        info.msgInfo.checkBox = checkBox;
    }
    info.msgInfo.buttonsText = textsByButton;

    return true;
}
