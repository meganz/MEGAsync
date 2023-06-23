#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <PlatformStrings.h>
#include <QMegaMessageBox.h>

#include "mega/types.h"

#include <QMessageBox>
#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> originalstall, QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    originalStall(originalstall),
    ui(new Ui::LocalAndRemoteDifferentWidget)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

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
    auto issue = getData();

    if(issue.consultData()->consultLocalData())
    {
        ui->chooseLocalCopy->setIssueSolved(issue.consultData()->isSolved());
        ui->chooseLocalCopy->updateUi(issue.consultData()->consultLocalData());

        ui->chooseLocalCopy->show();
    }
    else
    {
        ui->chooseLocalCopy->hide();
    }

    if(issue.consultData()->consultCloudData())
    {
        ui->chooseRemoteCopy->setIssueSolved(issue.consultData()->isSolved());
        ui->chooseRemoteCopy->updateUi(issue.consultData()->consultCloudData());

        ui->chooseRemoteCopy->show();
    }
    else
    {
        ui->chooseRemoteCopy->hide();
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;

    if(localInfo.isFile())
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>local file</b> %1?").arg(ui->chooseLocalCopy->data()->getFileName());
    }
    else
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>local folder</b> %1?").arg(ui->chooseLocalCopy->data()->getFileName());
    }

    if(node->isFile())
    {
        msgInfo.informativeText = tr("The <b>remote file</b> %1 will be moved to MEGA Rubbish Bin along with its versions.<br>You will be able to retrieve the file and its versions from there.</br>").arg(localInfo.fileName());
    }
    else
    {
        msgInfo.informativeText = tr("The <b>remote folder</b> %1 will be moved to MEGA Rubbish Bin.<br>You will be able to retrieve the folder from there.</br>").arg(localInfo.fileName());
    }

    msgInfo.finishFunc = [this](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::AcceptRole)
        {
            mUtilities.removeRemoteFile(ui->chooseRemoteCopy->data()->getFilePath());
            MegaSyncApp->getStalledIssuesModel()->solveIssue(false, getCurrentIndex());

            // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
            MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

            refreshUi();
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;

    if(node->isFile())
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>remote file</b> %1?").arg(ui->chooseRemoteCopy->data()->getFileName());
    }
    else
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>remote folder</b> %1?").arg(ui->chooseRemoteCopy->data()->getFileName());
    }

    if(localInfo.isFile())
    {
        msgInfo.informativeText = tr("The <b>local file</b> %1 will be moved to OS %2").arg(localInfo.fileName(), PlatformStrings::bin());
    }
    else
    {
        msgInfo.informativeText = tr("The <b>local folder</b> %1 will be moved to OS %2").arg(localInfo.fileName(), PlatformStrings::bin());
    }

    msgInfo.finishFunc = [this](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::AcceptRole)
        {
            mUtilities.removeLocalFile(ui->chooseLocalCopy->data()->getNativeFilePath());
            MegaSyncApp->getStalledIssuesModel()->solveIssue(true, getCurrentIndex());

            // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
            MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

            refreshUi();
        }
    };

    QMegaMessageBox::warning(msgInfo);
}
