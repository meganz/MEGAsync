#include "BugReportDialog.h"

// clang-format off
#include "DialogOpener.h"
#include "Preferences.h"
#include "QTMegaApiManager.h"
#include "RequestListenerManager.h"
#include "ui_BugReportDialog.h"
#include "ui_ProgressIndicatorDialog.h"

#include <QCloseEvent>
#include <QMegaMessageBox.h>
#include <QRegExp>
#include <TransfersModel.h>
// clang-format on

using namespace mega;

BugReportDialog::BugReportDialog(QWidget* parent, MegaSyncLogger& logger):
    QDialog(parent),
    logger(logger),
    ui(new Ui::BugReportDialog),
    mTransferFinished(false),
    mTransferError(MegaError::API_OK),
    mHadGlobalPause(false)
{
    ui->setupUi(this);

    ui->lDescribeBug->setText(
        ui->lDescribeBug->text() +
        QString::fromUtf8("<span style=\"color:red; text-decoration:none;\">*</span>"));
    ui->bSubmit->setDefault(true);
    ui->bSubmit->setEnabled(false);

    connect(ui->teDescribeBug, SIGNAL(textChanged()), this, SLOT(onDescriptionChanged()));
    connect(&logger, SIGNAL(logReadyForReporting()), this, SLOT(onReadyForReporting()));

    currentTransfer = 0;
    warningShown = false;
    errorShown = false;
    preparing = false;
    totalBytes = 0;
    transferredBytes = 0;
    lastpermil = -3;

    megaApi = ((MegaApplication*)qApp)->getMegaApi();
    mDelegateTransferListener = std::make_unique<QTMegaTransferListener>(megaApi, this);
}

BugReportDialog::~BugReportDialog()
{
    if (QTMegaApiManager::isMegaApiValid(megaApi))
    {
        // Just in case the dialog is closed from an exit action
        cancelCurrentReportUpload();
    }

    delete ui;
}

void BugReportDialog::onTransferStart(MegaApi*, MegaTransfer* transfer)
{
    if (!currentTransfer)
    {
        currentTransfer = transfer->getTag();
    }

    totalBytes = transfer->getTotalBytes();
    mTransferError = MegaError::API_OK;
    mTransferFinished = false;

    mProgressIndicatorDialog = new ProgressIndicatorDialog(this);

    connect(mProgressIndicatorDialog,
            &ProgressIndicatorDialog::cancelClicked,
            this,
            &BugReportDialog::cancelSendReport);

    mProgressIndicatorDialog->setDialogTitle(QLatin1String(" "));
    mProgressIndicatorDialog->setDialogDescription(
        tr("Bug report is uploading, it may take a few minutes"));
    mProgressIndicatorDialog->resetProgressBar();
    mProgressIndicatorDialog->setMinimumProgressBarValue(0);
    mProgressIndicatorDialog->setMaximumProgressBarValue(1010);
    mProgressIndicatorDialog->setProgressBarValue(0);
    lastpermil = 0;

    DialogOpener::showDialog(mProgressIndicatorDialog);
}

void BugReportDialog::onTransferUpdate(MegaApi*, MegaTransfer* transfer)
{
    if (mProgressIndicatorDialog && transfer->getState() == MegaTransfer::STATE_ACTIVE)
    {
        transferredBytes = transfer->getTransferredBytes();
        int permil =
            (totalBytes > 0) ? static_cast<int>((1000 * transferredBytes) / totalBytes) : 0;

        if (permil > lastpermil)
        {
            mProgressIndicatorDialog->setProgressBarValue(permil);
            lastpermil = permil;
        }
    }
}

void BugReportDialog::onTransferFinish(MegaApi*, MegaTransfer* transfer, MegaError* error)
{
    if (mHadGlobalPause)
    {
        MegaSyncApp->getTransfersModel()->setGlobalPause(true);
    }

    if (mProgressIndicatorDialog)
    {
        disconnect(mProgressIndicatorDialog,
                   &ProgressIndicatorDialog::cancelClicked,
                   this,
                   &BugReportDialog::cancelSendReport);

        mProgressIndicatorDialog->resetProgressBar();
    }

    totalBytes = 0;
    transferredBytes = 0;
    currentTransfer = 0;
    lastpermil = -3;

    mTransferError = error->getErrorCode();
    mTransferFinished = true;

    if (transfer->getState() == mega::MegaTransfer::STATE_CANCELLED)
    {
        preparing = false;
        warningShown = false;

        if (mProgressIndicatorDialog)
        {
            mProgressIndicatorDialog->close();
        }
    }
    else if (!warningShown)
    {
        postUpload();
    }

    logger.resumeAfterReporting();
}

void BugReportDialog::onTransferTemporaryError(MegaApi*, MegaTransfer*, MegaError* e)
{
    MegaApi::log(
        MegaApi::LOG_LEVEL_ERROR,
        QString::fromUtf8("Temporary error at report dialog: %1")
            .arg(QString::fromUtf8(
                mega::MegaError::getErrorString(e->getErrorCode(), mega::MegaError::API_EC_UPLOAD)))
            .toUtf8()
            .constData());
}

void BugReportDialog::onRequestFinish(MegaRequest* request, MegaError* e)
{
    switch (request->getType())
    {
        case MegaRequest::TYPE_SUPPORT_TICKET:
        {
            if (mProgressIndicatorDialog)
            {
                mProgressIndicatorDialog->close();
            }

            if (e->getErrorCode() == MegaError::API_OK)
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.parent = this->parentWidget();
                msgInfo.title = tr("Bug report");
                msgInfo.text = tr("Bug report success!");
                msgInfo.informativeText = tr("Your bug report has been submitted, a confirmation "
                                             "email will sent to you accordingly.");
                msgInfo.textFormat = Qt::RichText;
                msgInfo.buttons = QMessageBox::Ok;
                msgInfo.iconPixmap =
                    QPixmap(Utilities::getDevicePixelRatio() < 2 ?
                                QString::fromUtf8(":/images/bug_report_success.png") :
                                QString::fromUtf8(":/images/bug_report_success@2x.png"));

                accept();
                QMegaMessageBox::information(msgInfo);
            }
            else
            {
                showErrorMessage(e);
            }

            break;
        }
        default:
            break;
    }
}

void BugReportDialog::showErrorMessage(mega::MegaError* error)
{
    if (errorShown)
    {
        return;
    }

    errorShown = true;

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = tr("Bug report");
    msgInfo.text = tr("Error on submitting bug report");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok;
    msgInfo.finishFunc = [this](QPointer<QMessageBox>)
    {
        preparing = false;
        errorShown = false;
    };

    if (mTransferFinished && mTransferError == MegaError::API_EEXIST)
    {
        msgInfo.informativeText = tr("There is an ongoing report being uploaded.") +
                                  QString::fromUtf8("<br>") +
                                  tr("Please wait until the current upload is completed.");
        QMegaMessageBox::information(msgInfo);
    }
    else if (error && error->getErrorCode() == MegaError::API_ETOOMANY)
    {
        msgInfo.text = tr("You must wait 10 minutes before submitting another issue");
        msgInfo.informativeText =
            tr("Please try again later or contact our support team via [A]support@mega.co.nz[/A] "
               "if the problem persists.")
                .replace(
                    QString::fromUtf8("[A]"),
                    QString::fromUtf8("<span style=\"font-weight: bold; text-decoration:none;\">"))
                .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"));
        QMegaMessageBox::warning(msgInfo);
    }
    else
    {
        msgInfo.informativeText =
            tr("Bug report can't be submitted due to some error. Please try again or contact our "
               "support team via [A]support@mega.co.nz[/A]")
                .replace(
                    QString::fromUtf8("[A]"),
                    QString::fromUtf8("<span style=\"font-weight: bold; text-decoration:none;\">"))
                .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")) +
            QString::fromLatin1("\n");
        QMegaMessageBox::warning(msgInfo);
    }
}

void BugReportDialog::postUpload()
{
    if (mTransferError == MegaError::API_OK)
    {
        if (mProgressIndicatorDialog)
        {
            mProgressIndicatorDialog->setProgressBarValue(
                mProgressIndicatorDialog->getMaximumProgressBarValue());
        }
        createSupportTicket();
    }
    else
    {
        if (mProgressIndicatorDialog)
        {
            mProgressIndicatorDialog->close();
        }
        showErrorMessage();
    }
}

void BugReportDialog::createSupportTicket()
{
    QString report =
        QString::fromUtf8("Version string: %1   Version code: %2.%3   User-Agent: %4\n")
            .arg(Preferences::VERSION_STRING)
            .arg(Preferences::VERSION_CODE)
            .arg(Preferences::BUILD_ID)
            .arg(QString::fromUtf8(megaApi->getUserAgent()));

    report.append(
        QString::fromUtf8("Report filename: %1")
            .arg(reportFileName.isEmpty() ? QString::fromUtf8("Not sent") : reportFileName)
            .append(QString::fromUtf8("\n")));
    report.append(
        QString::fromUtf8("Title: %1").arg(ui->leTitleBug->text().append(QString::fromUtf8("\n"))));
    report.append(QString::fromUtf8("Description: %1")
                      .arg(ui->teDescribeBug->toPlainText().append(QString::fromUtf8("\n"))));

    auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
    megaApi->createSupportTicket(report.toUtf8().constData(), 6, listener.get());
}

void BugReportDialog::cancelCurrentReportUpload()
{
    if (!mTransferFinished && currentTransfer)
    {
        megaApi->cancelTransferByTag(currentTransfer);

        if (mHadGlobalPause)
        {
            MegaSyncApp->getTransfersModel()->setGlobalPause(true);
        }
    }
}

void BugReportDialog::on_bSubmit_clicked()
{
    if (preparing)
    {
        return;
    }

    if (logger.prepareForReporting())
    {
        preparing = true;
    }
}

void BugReportDialog::onReadyForReporting()
{
    reportFileName.clear();

    // If send log file is enabled
    if (ui->cbAttachLogs->isChecked())
    {
        QString pathToLogFile = Utilities::joinLogZipFiles(megaApi);
        if (pathToLogFile.isNull())
        {
            showErrorMessage();
            logger.resumeAfterReporting();
            preparing = false;
        }
        else
        {
            QFileInfo joinLogsFile{pathToLogFile};
            reportFileName = joinLogsFile.fileName();
            if (Preferences::instance()->getGlobalPaused())
            {
                mHadGlobalPause = true;
                MegaSyncApp->getTransfersModel()->setGlobalPause(false);
            }
            megaApi->startUploadForSupport(
                QDir::toNativeSeparators(pathToLogFile).toUtf8().constData(),
                true,
                mDelegateTransferListener.get());
        }
    }
    else
    {
        // Create support ticket
        createSupportTicket();
    }
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

    warningShown = true;

    MegaSyncApp->getTransfersModel()->pauseResumeTransferByTag(currentTransfer, true);
    if (mProgressIndicatorDialog)
    {
        mProgressIndicatorDialog->hide();
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = tr("Bug report");
    msgInfo.text = tr("Are you sure you want to exit uploading?");
    msgInfo.informativeText = tr("The bug report will not be submitted if you exit uploading.");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;

    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Yes, tr("Continue"));
    textsByButton.insert(QMessageBox::No, tr("Yes"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
    {
        if (msg->result() == QMessageBox::No)
        {
            if (mProgressIndicatorDialog)
            {
                mProgressIndicatorDialog->close();
            }

            cancelCurrentReportUpload();
            preparing = false;
            warningShown = false;
        }
        else
        {
            warningShown = false;
            if (mTransferFinished)
            {
                postUpload();
            }
            else if (currentTransfer && mProgressIndicatorDialog)
            {
                if (mProgressIndicatorDialog)
                {
                    mProgressIndicatorDialog->show();
                }
                mProgressIndicatorDialog->setProgressBarValue(lastpermil);
                MegaSyncApp->getTransfersModel()->pauseResumeTransferByTag(currentTransfer, false);
            }
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void BugReportDialog::onDescriptionChanged()
{
    ui->bSubmit->setEnabled(!ui->teDescribeBug->toPlainText().isEmpty());
}

void BugReportDialog::on_teDescribeBug_textChanged()
{
    if (ui->teDescribeBug->toPlainText().length() > mMaxDescriptionLength)
    {
        int diff = ui->teDescribeBug->toPlainText().length() -
                   mMaxDescriptionLength; // m_maxTextEditLength - just an integer
        QString newStr = ui->teDescribeBug->toPlainText();
        newStr.chop(diff);
        ui->teDescribeBug->setText(newStr);
        QTextCursor cursor(ui->teDescribeBug->textCursor());
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->teDescribeBug->setTextCursor(cursor);
    }
}
