#include "StreamingFromMegaDialog.h"
#include "ui_StreamingFromMegaDialog.h"
#include "NodeSelector.h"

#include "platform/Platform.h"
#include "control/Utilities.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

StreamingFromMegaDialog::StreamingFromMegaDialog(mega::MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StreamingFromMegaDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
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
    megaApi->httpServerStop();
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
    if (!event->spontaneous() || !selectedMegaNode)
    {
        event->accept();
        return;
    }

    QPointer<QMessageBox> msg = new QMessageBox(this);
    msg->setIcon(QMessageBox::Question);
    //        TO-DO: Uncomment when asset is included to the project
    //        msg->setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-question.png")
    //                                                            : QString::fromUtf8(":/images/mbox-question@2x.png")));

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
        event->accept();
    }
    else
    {
        event->ignore();
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
    MegaNode *node = megaApi->getNodeByHandle(nodeSelector->getSelectedFolderHandle());
    if (!node)
    {
        QMessageBox::warning(this, tr("Error"), tr("File not found"), QMessageBox::Ok);
        return;
    }

    if (selectedMegaNode)
    {
        delete selectedMegaNode;
    }
    selectedMegaNode = node;

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

        done(QDialog::Accepted);
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

    Preferences *preferences = Preferences::instance();
    QString lastPath = preferences->lastCustomStreamingApp();
    QFileInfo lastFile(lastPath);
    if (!lastPath.size() || !lastFile.exists())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Getting Applications path");
    #ifdef WIN32
        WCHAR buffer[MAX_PATH];
        if (SHGetFolderPath(0, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, buffer) == S_OK)
        {
            defaultPath = QString::fromUtf16((ushort *)buffer);
        }
    #else
        #ifdef __APPLE__
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);
            #else
                QStringList paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
                if (paths.size())
                {
                    defaultPath = paths.at(0);
                }
            #endif
        #else
            defaultPath = QString::fromUtf8("/usr/bin");
        #endif
    #endif
    }
    else
    {
        defaultPath = lastPath;
    }

    defaultPath = QDir::toNativeSeparators(defaultPath);
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Result: %1").arg(defaultPath).toUtf8().constData());

    QString path =  QFileDialog::getOpenFileName(0, tr("Choose application"), defaultPath);
    if (path.length() && !streamURL.isEmpty())
    {
        QFileInfo info(path);
        if (!info.exists())
        {
            return;
        }
        preferences->setLastCustomStreamingApp(path);
        openStreamWithApp(path);
    }
}

bool StreamingFromMegaDialog::generateStreamURL()
{
    if (!selectedMegaNode)
    {
        return false;
    }

    char *link = megaApi->httpServerGetLocalLink(selectedMegaNode);
    if (!link)
    {
        QMessageBox::warning(this, tr("Error"), tr("Error generating streaming link"), QMessageBox::Ok);
        return false;
    }
    streamURL = QString::fromUtf8(link);
    delete [] link;
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
            delete selectedMegaNode;
            selectedMegaNode = NULL;
            streamURL.clear();
        }
        else
        {
            updateFileInfo(name, CORRECT);
            generateStreamURL();
        }
    }
}

void StreamingFromMegaDialog::openStreamWithApp(QString app)
{
    if (app.isEmpty())
    {
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromEncoded(streamURL.toUtf8()));
        return;
    }

#ifndef __APPLE__
    QString command = QString::fromUtf8("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(app)).arg(streamURL);
    QProcess::startDetached(command);
#else
    QString args;
    args = QString::fromUtf8("-a ");
    args += QDir::toNativeSeparators(QString::fromUtf8("\"")+ app + QString::fromUtf8("\"")) + QString::fromAscii(" \"%1\"").arg(streamURL);
    QProcess::startDetached(QString::fromAscii("open ") + args);
#endif
}

void StreamingFromMegaDialog::updateFileInfo(QString fileName, linkstatus status)
{
    QFont f = ui->lFileName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideMiddle,ui->lFileName->maximumWidth()));
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
        ui->bCopyLink->setEnabled(true);
        ui->bCopyLink->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                       "stop: 0 rgba(246,247,250), stop: 1 rgba(232,233,235));"));
        break;
    default:
        statusIcon.addFile(QString::fromUtf8(":/images/streaming_off_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->bOpenDefault->setEnabled(false);
        ui->bOpenOther->setEnabled(false);
        ui->bCopyLink->setEnabled(false);
        ui->bCopyLink->setStyleSheet(QString());
        break;
    }

    ui->lState->setIcon(statusIcon);
    ui->lState->setIconSize(QSize(8, 8));
    ui->sFileInfo->setCurrentWidget(ui->pFileInfo);
}

void StreamingFromMegaDialog::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    switch (request->getType())
    {
    case MegaRequest::TYPE_GET_PUBLIC_NODE:
        if (e->getErrorCode() != MegaError::API_OK)
        {
            QMessageBox::warning(this, tr("Error"), tr("Error getting link information"), QMessageBox::Ok);
        }
        else
        {
            if (selectedMegaNode)
            {
                delete selectedMegaNode;
            }
            selectedMegaNode = request->getPublicMegaNode();
            onLinkInfoAvailable();
        }
        break;
    default:
        break;
    }
}
