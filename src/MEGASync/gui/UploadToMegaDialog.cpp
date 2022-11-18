#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"
#include "gui/NodeSelector.h"
#include "control/Utilities.h"
#include "MegaApplication.h"

#include <QPointer>

using namespace mega;

const char* UploadToMegaDialog::NODE_PATH_PROPERTY = "node_path";
const QString UploadToMegaDialog::DEFAULT_FOLDER_NAME = QLatin1String("MEGAsync Uploads");
const QString UploadToMegaDialog::DEFAULT_PATH = QLatin1String("/") + DEFAULT_FOLDER_NAME;

UploadToMegaDialog::UploadToMegaDialog(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadToMegaDialog)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    this->megaApi = megaApi;
    this->delegateListener = new QTMegaRequestListener(megaApi, this);

    selectedHandle = mega::INVALID_HANDLE;
    ui->eFolderPath->setText(DEFAULT_PATH);
    ui->eFolderPath->setProperty(NODE_PATH_PROPERTY, DEFAULT_PATH);
    ui->cDefaultPath->setChecked(false);
    ui->bChange->setEnabled(true);
    ui->bOK->setEnabled(true);
    ui->bOK->setDefault(true);
    highDpiResize.init(this);
}

UploadToMegaDialog::~UploadToMegaDialog()
{
    delete delegateListener;
    delete ui;
}

MegaHandle UploadToMegaDialog::getSelectedHandle()
{
    return selectedHandle;
}

bool UploadToMegaDialog::isDefaultFolder()
{
    return ui->cDefaultPath->isChecked();
}

void UploadToMegaDialog::setDefaultFolder(long long handle)
{
    std::unique_ptr<const char[]> pathStr(megaApi->getNodePathByNodeHandle(handle));
    if (pathStr)
    {
        QString path = QString::fromUtf8(pathStr.get());
        ui->eFolderPath->setProperty(NODE_PATH_PROPERTY, path);
        path.replace(QLatin1String("NO_KEY"), QCoreApplication::translate("MegaError", "Decryption error"));
        path.replace(QLatin1String("CRYPTO_ERROR"), QCoreApplication::translate("MegaError", "Decryption error"));
        ui->eFolderPath->setText(path);
    }

}

void UploadToMegaDialog::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    ui->bChange->setEnabled(true);
    ui->bOK->setEnabled(true);
    std::unique_ptr<MegaNode> node(megaApi->getNodeByHandle(request->getNodeHandle()));
    if (e->getErrorCode() != MegaError::API_OK || !node)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Request error: %1")
                     .arg(QCoreApplication::translate("MegaError", e->getErrorString())).toUtf8().constData());
        reject();
    }
    else
    {
        selectedHandle = node->getHandle();
        accept();
    }
}

void UploadToMegaDialog::on_bChange_clicked()
{
    showNodeSelector();
}

void UploadToMegaDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void UploadToMegaDialog::on_bOK_clicked()
{
    std::unique_ptr<MegaNode> node = getUploadFolder();
    if (!node || !node->isFolder())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Folder not found: %1").arg(ui->eFolderPath->property(NODE_PATH_PROPERTY).toString()).toUtf8().constData());
        ui->eFolderPath->setText(DEFAULT_PATH);
        ui->eFolderPath->setProperty(NODE_PATH_PROPERTY, DEFAULT_PATH);
    }
    else
    {
        selectedHandle = node->getHandle();
        accept();
    }
}

std::unique_ptr<MegaNode> UploadToMegaDialog::getUploadFolder()
{
    std::unique_ptr<MegaNode> node(megaApi->getNodeByPath(ui->eFolderPath->property(NODE_PATH_PROPERTY).toString().toUtf8().constData()));
    if(!node && ui->eFolderPath->property(NODE_PATH_PROPERTY).toString().compare(DEFAULT_PATH) == 0)
    {
        auto rootNode = ((MegaApplication*)qApp)->getRootNode();
        if (rootNode)
        {
            megaApi->createFolder(DEFAULT_FOLDER_NAME.toUtf8().constData(), rootNode.get(), delegateListener);
            //Disable the UI for the moment
            ui->bChange->setEnabled(false);
            ui->bOK->setEnabled(false);
        }
    }

    return std::move(node);
}

void UploadToMegaDialog::showNodeSelector()
{
    std::unique_ptr<NodeSelector> nodeSelector(new NodeSelector(NodeSelector::UPLOAD_SELECT, this));

    std::unique_ptr<MegaNode> defaultNode(megaApi->getNodeByPath(ui->eFolderPath->property(NODE_PATH_PROPERTY).toString().toUtf8().constData()));
    if (defaultNode)
    {
        nodeSelector->setSelectedNodeHandle(defaultNode->getHandle());
    }
    else
    {
        std::unique_ptr<MegaNode> rootNode(megaApi->getRootNode());
        nodeSelector->setSelectedNodeHandle(rootNode->getHandle());
    }

    int result = nodeSelector->exec();
    if (nodeSelector && result == QDialog::Accepted)
    {
        MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedNodeHandle();
        std::unique_ptr<const char[]> pathStr(megaApi->getNodePathByNodeHandle(selectedMegaFolderHandle));
        if (pathStr)
        {
            QString path = QString::fromUtf8(pathStr.get());
            ui->eFolderPath->setProperty(NODE_PATH_PROPERTY, path);
            path.replace(QLatin1String("NO_KEY"), QCoreApplication::translate("MegaError", "Decryption error"));
            path.replace(QLatin1String("CRYPTO_ERROR"), QCoreApplication::translate("MegaError", "Decryption error"));
            ui->eFolderPath->setText(path);
        }
    }
}
