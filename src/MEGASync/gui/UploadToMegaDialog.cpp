#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"
#include "gui/NodeSelector.h"
#include "control/Utilities.h"
#include "MegaApplication.h"

#include <QPointer>

using namespace mega;

const char* UploadToMegaDialog::NODE_PATH_PROPERTY = "node_path";
const QString UploadToMegaDialog::DEFAULT_PATH = QLatin1String("/MEGAsync Uploads");

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
    MegaNode *node = megaApi->getNodeByHandle(request->getNodeHandle());
    if (e->getErrorCode() != MegaError::API_OK || !node)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Request error: %1")
                     .arg(QCoreApplication::translate("MegaError", e->getErrorString())).toUtf8().constData());
        this->reject();
        delete node;
        return;
    }

    selectedHandle = node->getHandle();
    delete node;
    accept();
}

void UploadToMegaDialog::on_bChange_clicked()
{
    std::unique_ptr<NodeSelector> nodeSelector(new NodeSelector(NodeSelector::UPLOAD_SELECT, this));
    std::unique_ptr<MegaNode> defaultNode (megaApi->getNodeByPath(ui->eFolderPath->property(NODE_PATH_PROPERTY).toString().toUtf8().constData()));
    if (defaultNode)
    {
        nodeSelector->setSelectedNodeHandle(defaultNode->getHandle());
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
    MegaNode *node = megaApi->getNodeByPath(ui->eFolderPath->property(NODE_PATH_PROPERTY).toString().toUtf8().constData());
    if (node && node->isFolder())
    {
        selectedHandle = node->getHandle();
        delete node;
        accept();
        return;
    }
    delete node;
    if (!ui->eFolderPath->property(NODE_PATH_PROPERTY).toString().compare(DEFAULT_PATH))
    {
        ui->bChange->setEnabled(false);
        ui->bOK->setEnabled(false);
        auto rootNode = ((MegaApplication*)qApp)->getRootNode();
        if (!rootNode)
        {
            return;
        }

        megaApi->createFolder(QString::fromUtf8("MEGAsync Uploads").toUtf8().constData(), rootNode.get(), delegateListener);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Folder not found: %1").arg(ui->eFolderPath->property(NODE_PATH_PROPERTY).toString()).toUtf8().constData());
    ui->eFolderPath->setText(DEFAULT_PATH);
    ui->eFolderPath->setProperty(NODE_PATH_PROPERTY, DEFAULT_PATH);
    return;
}
