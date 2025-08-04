#include "StreamingFromMegaDialog.h"

#include "DialogOpener.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NodeSelectorSpecializations.h"
#include "Platform.h"
#include "QTMegaApiManager.h"
#include "ui_StreamingFromMegaDialog.h"
#include "Utilities.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QtConcurrent/QtConcurrent>

#include <regex>

#define MAX_STREAMING_BUFFER_SIZE 8242880 // 8 MB

//Streaming always first NODE = 0
const uint8_t StreamingFromMegaDialog::NODE_ID = 0;

using namespace mega;

StreamingFromMegaDialog::StreamingFromMegaDialog(mega::MegaApi *megaApi, mega::MegaApi* megaApiFolders, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::StreamingFromMegaDialog>())
    , mLinkProcessor(std::make_unique<LinkProcessor>(megaApi, megaApiFolders)) // Use make_unique here
    , lastStreamSelection{LastStreamingSelection::NOT_SELECTED}
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
    ui->bCopyLink->setDisabled(true);
    delegateTransferListener = std::make_unique<QTMegaTransferListener>(this->megaApi, this);
    megaApi->addTransferListener(delegateTransferListener.get());
    hideStreamingError();

    connect(mLinkProcessor.get(), &LinkProcessor::onLinkInfoRequestFinish, this, &StreamingFromMegaDialog::onLinkInfoAvailable);
}

StreamingFromMegaDialog::~StreamingFromMegaDialog()
{
    if (QTMegaApiManager::isMegaApiValid(megaApi))
    {
        megaApi->httpServerStop();
    }
}

bool StreamingFromMegaDialog::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    return QDialog::event(event);
}

void StreamingFromMegaDialog::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !mSelectedMegaNode)
    {
        event->accept();
        QDialog::closeEvent(event);
        return;
    }

    event->ignore();

    MessageDialogInfo msgInfo;
    msgInfo.titleText = tr("Stream from MEGA");
    msgInfo.descriptionText = tr("Are you sure that you want to stop the streaming?");
    msgInfo.parent = this;
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            accept();
        }
    };
    MessageDialogOpener::question(msgInfo);
}

void StreamingFromMegaDialog::on_bFromCloud_clicked()
{
    StreamNodeSelector* nodeSelector = new StreamNodeSelector(this);
    nodeSelector->setWindowTitle(tr("Select items"));
    nodeSelector->setSelectedNodeHandle(mSelectedMegaNode);

    DialogOpener::showDialog<NodeSelector>(nodeSelector, [nodeSelector, this]()
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
    inputDialog->setWindowTitle(tr("Open link"));
    inputDialog->setLabelText(tr("Enter a MEGA file link:"));
    inputDialog->resize(470, inputDialog->height());

    DialogOpener::showDialog<QInputDialog>(inputDialog, [inputDialog, this]()
    {
        if (inputDialog->result() == QDialog::Accepted)
        {
            if (isFolderLink(inputDialog->textValue()))
            {
                showErrorMessage(tr("Folder links can't be streamed"));
            }
            else
            {
                mPublicLink = inputDialog->textValue();
                requestNodeToLinkProcessor();
            }
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

    mLinkProcessor->resetAndSetLinkList(QStringList() << mPublicLink);

    updateFileInfo(QString(),LinkStatus::LOADING);

    mLinkProcessor->requestLinkInfo();
}

bool StreamingFromMegaDialog::isFolderLink(const QString& link) const
{
    auto folderLinkRegexp = std::regex(R"(^https://mega\.nz/folder/.*$)");
    return std::regex_match(link.trimmed().toUtf8().constData(), folderLinkRegexp);
}

void StreamingFromMegaDialog::showErrorMessage(const QString& message)
{
    MessageDialogInfo msgInfo;
    msgInfo.descriptionText = message;
    msgInfo.parent = this;
    MessageDialogOpener::warning(msgInfo);
}

//Connected only to onLinkImportFinish as only one link makes sense on streaming and it is always the first one
//As the LinkProcessor is removed, it should be done when the LinkProcessor finishes the LinkInfo available and not before (with
//the linkInfoAvailable, for example)
void StreamingFromMegaDialog::onLinkInfoAvailable()
{
    mLinkProcessor->onLinkSelected(NODE_ID, true);
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
        MessageDialogInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.descriptionText = tr("Error getting link information");
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
        {
            //This deletes the LinkProcess
            ui->sFileInfo->setCurrentWidget(ui->pNothingSelected);
            streamURL.clear();
            ui->bCopyLink->setDisabled(true);
        };
        MessageDialogOpener::warning(msgInfo);
    }
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

    MessageDialogInfo msgInfo;
    msgInfo.titleText = tr("Stream from MEGA");
    msgInfo.descriptionText = tr("Are you sure that you want to stop the streaming?");
    msgInfo.parent = this;
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            done(QDialog::Accepted);
        }
    };
    MessageDialogOpener::question(msgInfo);
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
                QStringList paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
                if (paths.size())
                {
                    defaultPath = paths.at(0);
                }
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

    std::unique_ptr<char[]> link(megaApi->httpServerGetLocalLink(mSelectedMegaNode.get()));
    if (!link)
    {
        showErrorMessage(tr("Error generating streaming link"));
        return false;
    }
    else
    {
        streamURL = QString::fromUtf8(link.get());
        return true;
    }
}

void StreamingFromMegaDialog::openStreamWithApp(QString app)
{
    if (app.isEmpty())
    {
        Utilities::openUrl(QUrl::fromEncoded(streamURL.toUtf8()));
        return;
    }

    Platform::getInstance()->streamWithApp(app, streamURL);
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
        MessageDialogInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.descriptionText = tr("File not found");
        MessageDialogOpener::warning(msgInfo);
    }
    else
    {
        mSelectedMegaNode = std::shared_ptr<MegaNode>(node);
        if(generateStreamURL())
        {
            lastStreamSelection = LastStreamingSelection::FROM_LOCAL_NODE;
            updateFileInfo(MegaNodeNames::getNodeName(node), LinkStatus::CORRECT);
            hideStreamingError();
        }
    }
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
        ui->lFileSize->setText(Utilities::getSizeString(mSelectedMegaNode->getSize()));

        QIcon typeIcon = Utilities::getExtensionPixmap(fileName, Utilities::AttributeType::medium);

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
                                                           "stop: 0 rgb(246,247,250), stop: 1 rgb(232,233,235));"));
        }
        else if(LinkStatus::TRANSFER_OVER_QUOTA == status)
        {
           statusIcon.addFile(QString::fromUtf8(":/images/streaming_error_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bOpenDefault->setEnabled(true);
            ui->bOpenOther->setEnabled(true);
            ui->bCopyLink->setEnabled(true);
            ui->bCopyLink->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                           "stop: 0 rgb(246,247,250), stop: 1 rgb(232,233,235));"));
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
        ui->bCopyLink->setEnabled(true);
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
