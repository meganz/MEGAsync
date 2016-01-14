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

    QString message = selectedMegaNode ? tr("Are you sure that you want to stop the streaming of \"%1\"?").arg(QString::fromUtf8(selectedMegaNode->getName()))
                                       : tr("Are you sure that you want to stop the streaming ?");

    event->ignore();
    QPointer<QMessageBox> msg = new QMessageBox(this);
    msg->setIcon(QMessageBox::Question);
    msg->setWindowTitle(tr("Stream from MEGA"));
    msg->setText(message);
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
        QMessageBox::warning(this, tr("Error"), tr("Error retrieving file"), QMessageBox::Ok);
        return;
    }
    updateFileInfo(QString::fromUtf8(selectedMegaNode->getName()), CORRECT);
    generateStreamURL();
    delete nodeSelector;
}
void StreamingFromMegaDialog::on_bFromPublicLink_clicked()
{
    QPointer<QInputDialog> id = new QInputDialog(this);
    id->setWindowTitle(tr("Import link"));
    id->setLabelText(tr("Enter a MEGA file link :"));
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
    QString message = selectedMegaNode ? tr("Are you sure that you want to stop the streaming of \"%1\"?").arg(QString::fromUtf8(selectedMegaNode->getName()))
                                       : tr("Are you sure that you want to stop the streaming ?");
    if (QMessageBox::question(this,
                             QString::fromUtf8("Stream from MEGA"),
                             message,
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
        QMessageBox::warning(this, tr("Error"), tr("Error generating Streaming Link"), QMessageBox::Ok);
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
        QMessageBox::warning(this, tr("Error"), tr("Error opening with default application. Please, use choose an application option"), QMessageBox::Ok);
        return;
    }
    QString args;
    args = QLatin1String("-a ");
    args += QDir::toNativeSeparators(QString::fromUtf8("\"")+ app + QString::fromUtf8("\"")) + QString::fromAscii(" \"%1\"").arg(streamURL);
    QProcess::startDetached(QString::fromAscii("open ") + args);

}

void StreamingFromMegaDialog::updateFileInfo(QString fileName, linkstatus status)
{

    /*QFont f = ui->lFileName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideMiddle, ui->lFileName->width()));
    ui->lFileName->adjustSize();*/
    ui->lFileName->setText(fileName);
    ui->lFileSize->setText(Utilities::getSizeString(selectedMegaNode->getSize()));

    QIcon typeIcon;
    typeIcon.addFile(Utilities::getExtensionPixmapMedium(fileName), QSize(), QIcon::Normal, QIcon::Off);

#ifdef __APPLE__
    ui->lFileType->setIcon(typeIcon);
    ui->lFileType->setIconSize(QSize(48, 48));
#else
    ui->lFileType->setPixmap(typeIcon.pixmap(QSize(48, 48)));
#endif

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

#ifdef __APPLE__
    ui->lState->setIcon(statusIcon);
    ui->lState->setIconSize(QSize(8, 8));
#else
    ui->lState->setPixmap(statusIcon.pixmap(QSize(8, 8)));
#endif

    ui->sFileInfo->setCurrentWidget(ui->pFileInfo);
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

