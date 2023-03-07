#include "mega/types.h"
#include "StreamingFromMegaDialog.h"
#include "ui_StreamingFromMegaDialog.h"
#include "node_selector/gui/NodeSelector.h"
#include "DialogOpener.h"

#include "QMegaMessageBox.h"
#include "platform/Platform.h"
#include "control/Utilities.h"
#include <MegaNodeNames.h>

#include <QCloseEvent>
#include <QInputDialog>


#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

#define MAX_STREAMING_BUFFER_SIZE 8242880 // 8 MB

//Streaming always first NODE = 0
const uint8_t StreamingFromMegaDialog::NODE_ID = 0;

using namespace mega;

StreamingFromMegaDialog::StreamingFromMegaDialog(mega::MegaApi *megaApi, mega::MegaApi* megaApiFolders, QWidget *parent) :
    QDialog(parent),
    ui(::mega::make_unique<Ui::StreamingFromMegaDialog>()),
    lastStreamSelection{LastStreamingSelection::NOT_SELECTED}
{
    ui->setupUi(this);
    Qt::WindowFlags flags =  Qt::Window | Qt::WindowSystemMenuHint
                                | Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);

    this->megaApi = megaApi;
    this->mMegaApiFolders = megaApiFolders;
    this->megaApi->httpServerSetMaxBufferSize(MAX_STREAMING_BUFFER_SIZE);

    int port = 4443;
    while (!megaApi->httpServerStart(true, port) && port < 4448)
    {
        port++;
    }

    setWindowTitle(tr("Stream from MEGA"));
    ui->bCopyLink->setEnabled(false);
    ui->sFileInfo->setCurrentWidget(ui->pNothingSelected);
    delegateTransferListener = ::mega::make_unique<QTMegaTransferListener>(this->megaApi, this);
    megaApi->addTransferListener(delegateTransferListener.get());
    hideStreamingError();
}

StreamingFromMegaDialog::~StreamingFromMegaDialog()
{
    megaApi->httpServerStop();
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
    if (!event->spontaneous() || !mSelectedMegaNode)
    {
        event->accept();
        QDialog::closeEvent(event);
        return;
    }

    if (QMegaMessageBox::question(this, tr("Stream from MEGA"), tr("Are you sure that you want to stop the streaming?"),QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

    QDialog::closeEvent(event);
}

void StreamingFromMegaDialog::on_bFromCloud_clicked()
{
    const QPointer<StreamNodeSelector> nodeSelector = new StreamNodeSelector(this);
    nodeSelector->setWindowTitle(tr("Select items"));
    nodeSelector->setSelectedNodeHandle(mSelectedMegaNode);

    DialogOpener::showDialog<StreamNodeSelector>(nodeSelector, [nodeSelector, this]()
    {
        if (nodeSelector->result() == QDialog::Accepted)
        {
            MegaNode *node = megaApi->getNodeByHandle(nodeSelector->getSelectedNodeHandle());
            updateFileInfoFromNode(node);
        }
    });
}

void StreamingFromMegaDialog::on_bFromPublicLink_clicked()
{
    const QPointer<QInputDialog> inputDialog = new QInputDialog(this);
    inputDialog->setWindowFlags(inputDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    inputDialog->setWindowTitle(tr("Open link"));
    inputDialog->setLabelText(tr("Enter a MEGA file link:"));
    inputDialog->resize(470, inputDialog->height());

    DialogOpener::showDialog<QInputDialog>(inputDialog, [inputDialog, this]()
    {
        if (inputDialog->result() == QDialog::Accepted)
        {
            mPublicLink = inputDialog->textValue();
            requestNodeToLinkProcessor();
        }
    });
}

void StreamingFromMegaDialog::requestNodeToLinkProcessor()
{
    QString url{mPublicLink.trimmed()};
    if (url.isEmpty())
    {
        return;
    }

    mLinkProcessor.reset(new LinkProcessor(QStringList() << mPublicLink, megaApi, mMegaApiFolders));
    connect(mLinkProcessor.get(), &LinkProcessor::onLinkInfoRequestFinish, this, &StreamingFromMegaDialog::onLinkInfoAvailable);

    updateFileInfo(QString(),LinkStatus::LOADING);

    mLinkProcessor->requestLinkInfo();
}

//Connected only to onLinkImportFinish as only one link makes sense on streaming and it is always the first one
//As the LinkProcessor is removed, it should be done when the LinkProcessor finishes the LinkInfo available and not before (with
//the linkInfoAvailable, for example)
void StreamingFromMegaDialog::onLinkInfoAvailable()
{
    mLinkProcessor->setSelected(NODE_ID, true);
    this->mSelectedMegaNode = std::shared_ptr<MegaNode>(mLinkProcessor->getNode(NODE_ID));

    if (mSelectedMegaNode)
    {
        updateFileInfo(MegaNodeNames::getNodeName(mSelectedMegaNode.get()),
                       mSelectedMegaNode->isNodeKeyDecrypted() ? LinkStatus::CORRECT : LinkStatus::WARNING);
        if (!mSelectedMegaNode->isNodeKeyDecrypted())
        {
            streamURL.clear();
        }
        else
        {
            generateStreamURL();
        }
    }
    else
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("Error getting link information"), QMessageBox::Ok);
        ui->sFileInfo->setCurrentWidget(ui->pNothingSelected);
    }

    //This deletes the LinkProcess
    mLinkProcessor.reset(nullptr);
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
    if (!mSelectedMegaNode)
    {
        close();
        return;
    }

    QPointer<StreamingFromMegaDialog> currentDialog = this;
    if (QMegaMessageBox::question(nullptr,
                             tr("Stream from MEGA"),
                             tr("Are you sure that you want to stop the streaming?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            done(QDialog::Accepted);
        }
    }
}

void StreamingFromMegaDialog::on_bOpenDefault_clicked()
{
    if (streamURL.isEmpty())
    {
        return;
    }

    QFileInfo fi(streamURL);
    QString app = Platform::getInstance()->getDefaultOpenApp(fi.suffix());
    openStreamWithApp(app);
}

void StreamingFromMegaDialog::on_bOpenOther_clicked()
{
    QString defaultPath;

    auto preferences = Preferences::instance();
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

    QPointer<StreamingFromMegaDialog> dialog(this);
    QString path = QDir::toNativeSeparators(QFileDialog::getOpenFileName(0, tr("Choose application"), defaultPath));
    if (dialog && path.length() && !streamURL.isEmpty())
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

void StreamingFromMegaDialog::updateStreamingState()
{
    if(lastStreamSelection == LastStreamingSelection::FROM_PUBLIC_NODE)
    {
        requestPublicNodeInfo();
    }
    else if(lastStreamSelection == LastStreamingSelection::FROM_LOCAL_NODE)
    {
        MegaNode *node = megaApi->getNodeByHandle(mSelectedMegaNode->getHandle());
        updateFileInfoFromNode(node);
    }
}

bool StreamingFromMegaDialog::generateStreamURL()
{
    if (!mSelectedMegaNode)
    {
        return false;
    }

    char *link = megaApi->httpServerGetLocalLink(mSelectedMegaNode.get());
    if (!link)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("Error generating streaming link"), QMessageBox::Ok);
        return false;
    }
    streamURL = QString::fromUtf8(link);
    delete [] link;
    return true;
}

void StreamingFromMegaDialog::openStreamWithApp(QString app)
{
    if (app.isEmpty())
    {
        Utilities::openUrl(QUrl::fromEncoded(streamURL.toUtf8()));
        return;
    }

#ifndef __APPLE__
#ifdef _WIN32
    QString command = QString::fromUtf8("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(app)).arg(streamURL);
#else
    QString command = QString::fromUtf8("%1 \"%2\"").arg(QDir::toNativeSeparators(app)).arg(streamURL);
#endif
    QProcess::startDetached(command);
#else
    QString args;
    args = QString::fromUtf8("-a ");
    args += QDir::toNativeSeparators(QString::fromUtf8("\"")+ app + QString::fromUtf8("\"")) + QString::fromAscii(" \"%1\"").arg(streamURL);
    QProcess::startDetached(QString::fromAscii("open ") + args);
#endif
}

void StreamingFromMegaDialog::showStreamingError()
{
    ui->toolButtonError->setVisible(true);
    ui->labelError->setVisible(true);
}

void StreamingFromMegaDialog::hideStreamingError()
{
    ui->toolButtonError->setVisible(false);
    ui->labelError->setVisible(false);
}

void StreamingFromMegaDialog::updateFileInfoFromNode(MegaNode *node)
{
    if (!node)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("File not found"), QMessageBox::Ok);
        return;
    }
    lastStreamSelection = LastStreamingSelection::FROM_LOCAL_NODE;
    mSelectedMegaNode = std::shared_ptr<MegaNode>(node);
    updateFileInfo(MegaNodeNames::getNodeName(node), LinkStatus::CORRECT);
    generateStreamURL();
    hideStreamingError();
}

void StreamingFromMegaDialog::requestPublicNodeInfo()
{
    requestNodeToLinkProcessor();
    hideStreamingError();
}

void StreamingFromMegaDialog::updateFileInfo(QString fileName, LinkStatus status)
{
    if(LinkStatus::LOADING == status)
    {
        ui->sFileInfo->setCurrentWidget(ui->pWaitingForFileInfo);
    }
    else
    {
        ui->lFileName->ensurePolished();
        ui->lFileName->setText(ui->lFileName->fontMetrics().elidedText(fileName,Qt::ElideMiddle,ui->lFileName->maximumWidth()));
        ui->lFileSize->setText(Utilities::getSizeString(static_cast<unsigned long long>(mSelectedMegaNode->getSize())));

        QIcon typeIcon = Utilities::getExtensionPixmapMedium(fileName);

        ui->lFileType->setIcon(typeIcon);
        ui->lFileType->setIconSize(QSize(48, 48));

        QIcon statusIcon;

        if(LinkStatus::CORRECT == status)
        {
            statusIcon.addFile(QString::fromUtf8(":/images/streaming_on_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bOpenDefault->setEnabled(true);
            ui->bOpenOther->setEnabled(true);
            ui->bCopyLink->setEnabled(true);
            ui->bCopyLink->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                           "stop: 0 rgba(246,247,250), stop: 1 rgba(232,233,235));"));
        }
        else if(LinkStatus::TRANSFER_OVER_QUOTA == status)
        {
           statusIcon.addFile(QString::fromUtf8(":/images/streaming_error_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bOpenDefault->setEnabled(true);
            ui->bOpenOther->setEnabled(true);
            ui->bCopyLink->setEnabled(true);
            ui->bCopyLink->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                           "stop: 0 rgba(246,247,250), stop: 1 rgba(232,233,235));"));
        }
        else
        {
            statusIcon.addFile(QString::fromUtf8(":/images/streaming_off_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bOpenDefault->setEnabled(false);
            ui->bOpenOther->setEnabled(false);
            ui->bCopyLink->setEnabled(false);
            ui->bCopyLink->setStyleSheet(QString());
        }

        ui->lState->setIcon(statusIcon);
        ui->lState->setIconSize(QSize(8, 8));
        ui->sFileInfo->setCurrentWidget(ui->pFileInfo);
    }
}

void StreamingFromMegaDialog::onTransferTemporaryError(mega::MegaApi*, mega::MegaTransfer* transfer, mega::MegaError* e)
{
    const bool errorIsOverQuota{e->getErrorCode() == MegaError::API_EOVERQUOTA};
    if(transfer->isStreamingTransfer() && errorIsOverQuota)
    {
        updateFileInfo(MegaNodeNames::getNodeName(mSelectedMegaNode.get()), LinkStatus::TRANSFER_OVER_QUOTA);
        showStreamingError();

        show();
        raise();
        activateWindow();
    }
}
