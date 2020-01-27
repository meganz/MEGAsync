#include "BugReportDialog.h"
#include <QCloseEvent>
#include <QRegExp>
#include "ui_BugReportDialog.h"
#include "control/gzjoin.h"

using namespace mega;

BugReportDialog::BugReportDialog(QWidget *parent, MegaSyncLogger& logger) :
    QDialog(parent),
    logger(logger),
    ui(new Ui::BugReportDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lDescribeBug->setText(ui->lDescribeBug->text() + QString::fromUtf8("<span style=\"color:red; text-decoration:none;\">*</span>"));
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

    megaApi = ((MegaApplication *)qApp)->getMegaApi();
    delegateTransferListener = new QTMegaTransferListener(megaApi, this);
    delegateRequestListener = new QTMegaRequestListener(megaApi, this);
    highDpiResize.init(this);
}

BugReportDialog::~BugReportDialog()
{
    delete ui;
    delete delegateTransferListener;
    delete delegateRequestListener;
}

void BugReportDialog::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    if (!currentTransfer)
    {
        currentTransfer = transfer->getTag();
    }

    totalBytes = transfer->getTotalBytes();

    sendProgress.reset(new QProgressDialog(this));
    sendProgress->setWindowFlags(sendProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(sendProgress.get(), SIGNAL(canceled()), this, SLOT(cancelSendReport()));

    sendProgress->setWindowModality(Qt::WindowModal);
    sendProgress->setLabelText(tr("Bug report is uploading, it may take a few minutes"));
    sendProgress->setMinimumDuration(0);
    sendProgress->setMinimum(0);
    sendProgress->setMaximum(1010);
    sendProgress->setValue(0);
    sendProgress->setAutoClose(false);
    sendProgress->setAutoReset(false);
    lastpermil = 0;
    sendProgress->show();
}

void BugReportDialog::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    transferredBytes = transfer->getTransferredBytes();
    int permil = (totalBytes > 0) ? ((1000 * transferredBytes) / totalBytes) : 0;
    if (!warningShown)
    {
        if (permil != lastpermil)
        {
            sendProgress->setValue(permil);
            lastpermil = permil;
        }
    }
}

void BugReportDialog::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *error)
{
    sendProgress->reset();
    totalBytes = 0;
    transferredBytes = 0;
    currentTransfer = 0;
    lastpermil = -3;

    QMessageBox msgBox;
    HighDpiResize hDpiResizer(&msgBox);
    msgBox.setWindowTitle(tr("Bug report"));

    if (error->getErrorCode() == MegaError::API_OK)
    {
        sendProgress->setValue(sendProgress->maximum());

        createSupportTicket();
    }
    else if (error->getErrorCode() == MegaError::API_EEXIST)
    {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Error on submitting bug report"));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setInformativeText(tr("There is an ongoing report being uploaded.")
                                  + QString::fromUtf8("<br>") +
                                  tr("Please wait until the current upload is completed."));
        msgBox.addButton(tr("Ok"), QMessageBox::AcceptRole);
        msgBox.exec();
    }
    else
    {
        showErrorMessage();
    }

    if (error->getErrorCode() != MegaError::API_OK)
    {
        sendProgress->hide();
    }

    logger.resumeAfterReporting();
}

void BugReportDialog::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Temporary error at report dialog: %1")
                 .arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
}

void BugReportDialog::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_SUPPORT_TICKET:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                QMessageBox msgBox;
                HighDpiResize hDpiResizer(&msgBox);
                msgBox.setWindowTitle(tr("Bug report"));
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("Bug report success!"));
                msgBox.setTextFormat(Qt::RichText);
                msgBox.setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/bug_report_success.png")
                                                                                  : QString::fromUtf8(":/images/bug_report_success@2x.png")));
                msgBox.setInformativeText(tr("Your bug report has been submitted, a confirmation email will sent to you accordingly."));
                msgBox.addButton(tr("Ok"), QMessageBox::AcceptRole);
                msgBox.exec();
            }
            else
            {
                showErrorMessage();
            }

            if (sendProgress)
            {
                sendProgress->hide();
            }

            break;
        }
        default:
            break;
    }

    accept();
}

void BugReportDialog::showErrorMessage()
{
    if (errorShown)
    {
        return;
    }

    QMessageBox msgBox;
    HighDpiResize hDpiResizer(&msgBox);
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
    errorShown = true;
    msgBox.exec();
    errorShown = false;
}

void BugReportDialog::createSupportTicket()
{
    QString report = QString::fromUtf8("Version string: %1   Version code: %2.%3   User-Agent: %4\n").arg(Preferences::VERSION_STRING)
                                    .arg(Preferences::VERSION_CODE)
                                    .arg(Preferences::BUILD_ID)
                                    .arg(QString::fromUtf8(megaApi->getUserAgent()));

    report.append(QString::fromUtf8("Report filename: %1").arg(reportFileName.isEmpty() ? QString::fromUtf8("Not sent") : reportFileName).append(QString::fromUtf8("\n")));
    report.append(QString::fromUtf8("Title: %1").arg(ui->leTitleBug->text().append(QString::fromUtf8("\n"))));
    report.append(QString::fromUtf8("Description: %1").arg(ui->teDescribeBug->toPlainText().append(QString::fromUtf8("\n"))));

    megaApi->createSupportTicket(report.toUtf8().constData(), 6, delegateRequestListener);
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

    //If send log file is enabled
    if (ui->cbAttachLogs->isChecked())
    {
        QDir logDir{MegaApplication::applicationDataPath().append(QString::fromUtf8("/") + LOGS_FOLDER_LEAFNAME_QSTRING)};
        if (logDir.exists())
        {
            QString fileFormat{QDir::separator() + QString::fromUtf8("%1_%2").arg(QDateTime::currentDateTimeUtc().toString(QString::fromAscii("yyMMdd_hhmmss")))
                                                                         .arg(QString::fromUtf8(std::unique_ptr<MegaUser>(megaApi->getMyUser())->getEmail()))};

            QFileInfo joinLogsFile(logDir.absolutePath().append(fileFormat).append(QString::fromUtf8(".gz")));
#ifdef _WIN32
            FILE * pFile = nullptr;
            errno_t er = _wfopen_s(&pFile, joinLogsFile.absoluteFilePath().toStdWString().c_str(), L"a+b");
            if (er)
            {
                megaApi->log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error opening file for joining log zip files (%1) : %2")
                             .arg(er).arg(joinLogsFile.filePath()).toUtf8().constData());
                pFile = nullptr; //just in case
            }

#else
            FILE * pFile = fopen(joinLogsFile.absoluteFilePath().toUtf8().constData(), "a+b");
#endif
            if (!pFile)
            {
                std::cerr << "Error opening file for joining log zip files " << std::endl;
                megaApi->log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error opening file for joining log zip files: %1").arg(joinLogsFile.filePath()).toUtf8().constData());


                logger.resumeAfterReporting();
                preparing = false;
                return;
            }

            unsigned long crc, tot;
            gzinit(&crc, &tot, pFile);

            QFileInfoList logFiles = logDir.entryInfoList(QStringList() << QString::fromUtf8("MEGAsync.[0-9]*.log"), QDir::Files);
            int nLogFiles = logFiles.count();

            std::sort(logFiles.begin(), logFiles.end(), [](const QFileInfo &v1, const QFileInfo &v2){
                return v1.fileName().remove(QRegExp(QString::fromUtf8("[^\\d]"))).toInt() > v2.fileName().remove(QRegExp(QString::fromUtf8("[^\\d]"))).toInt();} );

            foreach (QFileInfo i, logFiles)
            {
                try
                {
#ifdef _WIN32
                    gzcopy(i.absoluteFilePath().toStdWString().c_str(), --nLogFiles, &crc, &tot, pFile);
#else
                    gzcopy(i.absoluteFilePath().toUtf8().constData(), --nLogFiles, &crc, &tot, pFile);
#endif
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error joining zip files for bug report " << e.what() << std::endl;
                    megaApi->log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error joining zip files for bug report : %1")
                                 .arg(QString::fromUtf8(e.what())).toUtf8().constData());

                    fclose(pFile);
                    QFile::remove(joinLogsFile.absoluteFilePath());
                    showErrorMessage();
                    logger.resumeAfterReporting();
                    preparing = false;
                    return;
                }
            }

            fclose(pFile);

            reportFileName = joinLogsFile.fileName();
            megaApi->startUploadForSupport(QDir::toNativeSeparators(joinLogsFile.absoluteFilePath()).toUtf8().constData(), true, delegateTransferListener);
        }
    }
    else
    {
        //Create support ticket
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

    QMessageBox msgBox;
    HighDpiResize hDpiResizer(&msgBox);
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
        preparing = false;
        reject();
    }
    else if (ret == QMessageBox::AcceptRole)
    {
        if (currentTransfer && sendProgress)
        {
            sendProgress->setValue(lastpermil);
        }
    }
}

void BugReportDialog::onDescriptionChanged()
{
    ui->bSubmit->setEnabled(ui->teDescribeBug->toPlainText().isEmpty() ? false : true);
}

void BugReportDialog::on_teDescribeBug_textChanged()
{
    if(ui->teDescribeBug->toPlainText().length() > mMaxDescriptionLength)
    {
        int diff = ui->teDescribeBug->toPlainText().length() - mMaxDescriptionLength; //m_maxTextEditLength - just an integer
        QString newStr = ui->teDescribeBug->toPlainText();
        newStr.chop(diff);
        ui->teDescribeBug->setText(newStr);
        QTextCursor cursor(ui->teDescribeBug->textCursor());
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->teDescribeBug->setTextCursor(cursor);
    }
}
