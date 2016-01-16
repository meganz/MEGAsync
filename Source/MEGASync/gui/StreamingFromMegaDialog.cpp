#include "StreamingFromMegaDialog.h"
#include "ui_StreamingFromMegaDialog.h"
#include "NodeSelector.h"

#include "platform/Platform.h"
#include "control/Utilities.h"

using namespace mega;

StreamingFromMegaDialog::StreamingFromMegaDialog(mega::MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StreamingFromMegaDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    Qt::WindowFlags flags =  Qt::Window | Qt::WindowSystemMenuHint
                                | Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);

    this->megaApi = megaApi;
    this->selectedMegaNode = NULL;
    this->megaApi->httpServerStart();

    setWindowTitle(tr("Stream from MEGA"));
    ui->bCopyLink->setEnabled(false);
    ui->sFileInfo->setCurrentWidget(ui->pNothingSelected);
    delegateListener = new QTMegaRequestListener(this->megaApi, this);
}

StreamingFromMegaDialog::~StreamingFromMegaDialog()
{
    delete ui;
    delete delegateListener;
    delete selectedMegaNode;
}

void StreamingFromMegaDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QDialog::changeEvent(event);
}

void StreamingFromMegaDialog::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous())
    {
        event->accept();
        return;
    }

    if (selectedMegaNode)
    {
        event->ignore();
        QPointer<QMessageBox> msg = new QMessageBox(this);
        msg->setIcon(QMessageBox::Question);
        msg->setWindowTitle(tr("Stream from MEGA"));
        msg->setText(tr("Are you sure that you want to stop the streaming?"));
        msg->addButton(QMessageBox::Yes);
        msg->addButton(QMessageBox::No);
        msg->setDefaultButton(QMessageBox::No);
        int button = msg->exec();
        if (msg)
        {
            delete msg;
        }

        if (button == QMessageBox::Yes)
        {
            megaApi->httpServerStop();
            event->accept();
        }
    }
    else
    {
        event->accept();
    }
}

void StreamingFromMegaDialog::on_bFromCloud_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(megaApi, NodeSelector::STREAM_SELECT, this);
    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }
    selectedMegaNode = megaApi->getNodeByHandle(nodeSelector->getSelectedFolderHandle());
    if (!selectedMegaNode)
    {
        QMessageBox::warning(this, tr("Error"), tr("File not found"), QMessageBox::Ok);
        return;
    }
    updateFileInfo(QString::fromUtf8(selectedMegaNode->getName()), CORRECT);
    generateStreamURL();
    delete nodeSelector;
}
void StreamingFromMegaDialog::on_bFromPublicLink_clicked()
{
    QPointer<QInputDialog> id = new QInputDialog(this);
    id->setWindowTitle(tr("Open link"));
    id->setLabelText(tr("Enter a MEGA file link:"));
    int result = id->exec();
    if (!id || !result)
    {
      delete id;
      return;
    }

    QString text = id->textValue();
    delete id;
    text = text.trimmed();
    if (text.isEmpty())
    {
        return;
    }
    megaApi->getPublicNode(text.toUtf8().constData(), delegateListener);

}
void StreamingFromMegaDialog::on_bCopyLink_clicked()
{
    if (!streamURL.isEmpty())
    {
        QApplication::clipboard()->setText(streamURL);
        ((MegaApplication *)qApp)->showInfoMessage(tr("The link has been copied to the clipboard"));
    }

}

void StreamingFromMegaDialog::on_bClose_clicked()
{
    if (!selectedMegaNode)
    {
        close();
        return;
    }

    if (QMessageBox::question(this,
                             tr("Stream from MEGA"),
                             tr("Are you sure that you want to stop the streaming?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {

        megaApi->httpServerStop();
        close();
    }
}

void StreamingFromMegaDialog::on_bOpenDefault_clicked()
{
    if (streamURL.isEmpty())
    {
        return;
    }

    QFileInfo fi(streamURL);
    QString app = Platform::getDefaultOpenApp(fi.suffix());
    openStreamWithApp(app);
}

void StreamingFromMegaDialog::on_bOpenOther_clicked()
{
    QString defaultPath;

#ifdef WIN32
    #if QT_VERSION < 0x050000
        defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);
    #else
        defaultPath = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)[0];
    #endif
#else
    #if QT_VERSION < 0x050000
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Getting Applications path (QT4)");
        defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Result: %1").arg(defaultPath).toUtf8().constData());
    #else
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Getting Applications path (QT5)");
        defaultPath = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)[0];
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Result: %1").arg(defaultPath).toUtf8().constData());
    #endif
#endif

    QString path =  QFileDialog::getOpenFileName(0, tr("Choose application"), defaultPath);
    if (path.length() && !streamURL.isEmpty())
    {
        QDir dir(path);
        if (!dir.exists())
        {
            return;
        }
        openStreamWithApp(path);
    }
}

bool StreamingFromMegaDialog::generateStreamURL()
{
    if (!selectedMegaNode)
    {
        selectedMegaNode = NULL;
        return false;
    }

    streamURL = QString::fromUtf8(megaApi->httpServerGetLocalLink(selectedMegaNode));
    if (streamURL.isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Error generating streaming link"), QMessageBox::Ok);
        return false;
    }

    return true;
}

void StreamingFromMegaDialog::onLinkInfoAvailable()
{
    if (selectedMegaNode)
    {
        QString name = QString::fromUtf8(selectedMegaNode->getName());
        if (!name.compare(QString::fromAscii("NO_KEY")) || !name.compare(QString::fromAscii("CRYPTO_ERROR")))
        {
            updateFileInfo(tr("Decryption error"), WARNING);
        }
        else
        {
            updateFileInfo(name, CORRECT);
        }
    }

}

void StreamingFromMegaDialog::openStreamWithApp(QString app)
{
    if (app.isEmpty())
    {
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(streamURL));
        return;
    }

#ifndef __APPLE__
    QString command = QString::fromUtf8("\"") + QDir::toNativeSeparators(app) + QString::fromUtf8("\"")
            + QString::fromUtf8(" ") + QString::fromAscii(" \"%1\"").arg(streamURL);
    QProcess::startDetached(command);
#else
    QString args;
    args = QLatin1String("-a ");
    args += QDir::toNativeSeparators(QString::fromUtf8("\"")+ app + QString::fromUtf8("\"")) + QString::fromAscii(" \"%1\"").arg(streamURL);
    QProcess::startDetached(QString::fromAscii("open ") + args);
#endif
}

void StreamingFromMegaDialog::updateFileInfo(QString fileName, linkstatus status)
{
    ui->lFileName->setText(fileName);
    ui->lFileSize->setText(Utilities::getSizeString(selectedMegaNode->getSize()));

    QIcon typeIcon;
    typeIcon.addFile(Utilities::getExtensionPixmapMedium(fileName), QSize(), QIcon::Normal, QIcon::Off);

    ui->lFileType->setIcon(typeIcon);
    ui->lFileType->setIconSize(QSize(48, 48));

    QIcon statusIcon;
    switch (status)
    {
    case LOADING:
        break;
    case CORRECT:
        statusIcon.addFile(QString::fromUtf8(":/images/streaming_on_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->bOpenDefault->setEnabled(true);
        ui->bOpenOther->setEnabled(true);
        break;
    default:
        statusIcon.addFile(QString::fromUtf8(":/streaming_off_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->bOpenDefault->setEnabled(false);
        ui->bOpenOther->setEnabled(false);
        break;
    }

    ui->lState->setIcon(statusIcon);
    ui->lState->setIconSize(QSize(8, 8));

    ui->sFileInfo->setCurrentWidget(ui->pFileInfo);
    ui->bCopyLink->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                   "stop: 0 rgba(246,247,250), stop: 1 rgba(232,233,235));"));
    ui->bCopyLink->setEnabled(true);
}

void StreamingFromMegaDialog::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    switch (request->getType())
    {
        case MegaRequest::TYPE_GET_PUBLIC_NODE:

        if (e->getErrorCode() != MegaError::API_OK)
        {
            selectedMegaNode = NULL;
            QMessageBox::warning(this, tr("Error"), tr("Error getting link information"), QMessageBox::Ok);
        }
        else
        {
            selectedMegaNode = request->getPublicMegaNode();
            onLinkInfoAvailable();
        }

            break;
        default:
            break;
    }

}

