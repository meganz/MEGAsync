#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>

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
    QMessageBox* msgBox = new QMessageBox(dialog->getDialog());
    msgBox->setWindowTitle(QString::fromUtf8("MEGAsync"));
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setText(tr("Are you sure you want to keep the local %1 %2?"));
    msgBox->setInformativeText(tr("The remote %1 %2 will be moved to the rubbish bin along with all its versions."));

    if(localInfo.isFile())
    {
        msgBox->setText(tr("Are you sure you want to keep the local file %1?").arg(ui->chooseLocalCopy->data()->getFileName()));
    }
    else
    {
        msgBox->setText(tr("Are you sure you want to keep the local folder %1?").arg(ui->chooseLocalCopy->data()->getFileName()));
    }

    if(node->isFile())
    {
        msgBox->setInformativeText(tr("The remote file %1 will be moved to the rubbish bin along with all its versions.").arg(localInfo.fileName()));
    }
    else
    {
        msgBox->setInformativeText(tr("The remote folder %1 will be moved to the rubbish bin along with all its versions.").arg(localInfo.fileName()));
    }

    msgBox->addButton(tr("Ok"), QMessageBox::AcceptRole);
    msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);

    DialogOpener::showDialog<QMessageBox>(msgBox, [this, msgBox]()
    {
        if(msgBox->result() == QDialogButtonBox::AcceptRole)
        {
            mUtilities.removeRemoteFile(ui->chooseRemoteCopy->data()->getFilePath());
            MegaSyncApp->getStalledIssuesModel()->solveIssue(false, getCurrentIndex());

            // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
            MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

            refreshUi();
        }
    });
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    QMessageBox* msgBox = new QMessageBox(dialog->getDialog());
    msgBox->setWindowTitle(QString::fromUtf8("MEGAsync"));
    msgBox->setIcon(QMessageBox::Warning);
    if(node->isFile())
    {
        msgBox->setText(tr("Are you sure you want to keep the remote file %1?").arg(ui->chooseRemoteCopy->data()->getFileName()));
    }
    else
    {
        msgBox->setText(tr("Are you sure you want to keep the remote folder %1?").arg(ui->chooseRemoteCopy->data()->getFileName()));
    }

    if(localInfo.isFile())
    {
        msgBox->setInformativeText(tr("The local file %1 will be moved to the rubbish bin").arg(localInfo.fileName()));
    }
    else
    {
        msgBox->setInformativeText(tr("The local folder %1 will be moved to the rubbish bin").arg(localInfo.fileName()));
    }

    msgBox->addButton(tr("Ok"), QMessageBox::AcceptRole);
    msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);

    DialogOpener::showDialog<QMessageBox>(msgBox, [this, msgBox]()
    {
        if(msgBox->result() == QDialogButtonBox::AcceptRole)
        {
            mUtilities.removeLocalFile(ui->chooseLocalCopy->data()->getNativeFilePath());
            MegaSyncApp->getStalledIssuesModel()->solveIssue(true, getCurrentIndex());

            // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
            MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

            refreshUi();
        }
    });
}
