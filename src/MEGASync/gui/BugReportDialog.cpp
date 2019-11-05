#include "BugReportDialog.h"
#include <QCloseEvent>
#include "ui_BugReportDialog.h"

using namespace mega;

BugReportDialog::BugReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BugReportDialog)
{
    ui->setupUi(this);

    ui->lDescribeBug->setText(ui->lDescribeBug->text() + QString::fromUtf8("<span style=\"color:red; text-decoration:none;\">*</span>"));
    ui->bSubmit->setDefault(true);
    ui->bSubmit->setEnabled(false);

    connect(ui->teDescribeBug, SIGNAL(textChanged()), this, SLOT(onDescriptionChanged()));

    currentTransfer = 0;
    warningShown = false;
    totalBytes = 0;
    transferredBytes = 0;

    sendProgress.setWindowModality(Qt::WindowModal);
    sendProgress.setLabelText(tr("Bug report is uploading, it may take a few minutes"));
    sendProgress.setMinimumDuration(0);
    sendProgress.setMinimum(0);
    sendProgress.setMaximum(1000);
    sendProgress.setAutoClose(true);

    connect(&sendProgress, SIGNAL(canceled()), this, SLOT(cancelSendReport()));

    megaApi = ((MegaApplication *)qApp)->getMegaApi();
    delegateListener = new QTMegaTransferListener(megaApi, this);
}

BugReportDialog::~BugReportDialog()
{
    delete ui;
    delete delegateListener;
}

void BugReportDialog::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    if (!currentTransfer)
    {
        currentTransfer = transfer->getTag();
    }

    totalBytes = transfer->getTotalBytes();
    sendProgress.show();
}

void BugReportDialog::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    transferredBytes = transfer->getTransferredBytes();
    unsigned int permil = (totalBytes > 0) ? ((1000 * transferredBytes) / totalBytes) : 0;
    if (!warningShown)
    {
        sendProgress.setValue(permil);
    }
}

void BugReportDialog::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *error)
{
    sendProgress.reset();
    totalBytes = 0;
    transferredBytes = 0;
    currentTransfer = 0;

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Bug report"));

    if (error->getErrorCode() == MegaError::API_OK)
    {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Bug report success!"));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setInformativeText(tr("Your bug report has been submitted, a confirmation email will sent to you accordingly."));
    }
    else if (error->getErrorCode() == MegaError::API_EEXIST)
    {
        //TODO: There is an ongoing bug report upload, show info message
    }
    else
    {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Error on submitting bug report"));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setInformativeText(
                    tr("Bug report can't be submitted due to some error. Please try again or contact our support team via [A]support@mega.co.nz[/A]")
                        .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\"font-weight: bold; text-decoration:none;\">"))
                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                         + QString::fromAscii("\n"));
    }

    msgBox.addButton(tr("Ok"), QMessageBox::AcceptRole);
    msgBox.exec();

    accept();
}

void BugReportDialog::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Bug report"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("Error on submitting bug report"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setInformativeText(
                tr("Bug report can't be submitted due to some error. Please try again or contact our support team via [A]support@mega.co.nz[/A]")
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\"font-weight: bold; text-decoration:none;\">"))
                    .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                     + QString::fromAscii("\n"));

    msgBox.addButton(tr("Ok"), QMessageBox::AcceptRole);
    msgBox.exec();
}

void BugReportDialog::on_bSubmit_clicked()
{
    //TODO: Call megaApi to submit bug report
}

void BugReportDialog::on_bCancel_clicked()
{
    reject();
}

void BugReportDialog::cancelSendReport()
{
    if (warningShown)
    {
        return;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Bug report"));
    msgBox.setText(tr("Are you sure you want to exit uploading?"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setInformativeText(tr("The bug report will not be submitted if you exit uploading."));

    msgBox.addButton(tr("Continue"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Yes"), QMessageBox::RejectRole);

    warningShown = true;
    int ret = msgBox.exec();
    warningShown = false;

    if (ret == QMessageBox::RejectRole)
    {
        if (currentTransfer)
        {
            megaApi->cancelTransferByTag(currentTransfer);
        }
        reject();
    }
}

void BugReportDialog::onDescriptionChanged()
{
    ui->bSubmit->setEnabled(ui->teDescribeBug->toPlainText().isEmpty() ? false : true);
}
