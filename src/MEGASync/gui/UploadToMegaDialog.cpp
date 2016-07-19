#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"
#include "gui/NodeSelector.h"
#include "control/Utilities.h"
#include <QPointer>

using namespace mega;

UploadToMegaDialog::UploadToMegaDialog(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadToMegaDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->megaApi = megaApi;
    this->delegateListener = new QTMegaRequestListener(megaApi, this);

    selectedHandle = mega::INVALID_HANDLE;
    ui->eFolderPath->setText(QString::fromUtf8("/MEGAsync Uploads"));
    ui->cDefaultPath->setChecked(false);
    ui->bChange->setEnabled(true);
    ui->bOK->setEnabled(true);
    ui->bOK->setDefault(true);
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
    MegaNode *node = megaApi->getNodeByHandle(handle);
    if (!node)
    {
        return;
    }

    const char *path = megaApi->getNodePath(node);
    if (!path)
    {
        delete node;
        return;
    }

    ui->eFolderPath->setText(QString::fromUtf8(path));
    delete [] path;
    delete node;
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
    QPointer<NodeSelector> nodeSelector = new NodeSelector(megaApi, NodeSelector::UPLOAD_SELECT, this);
    MegaNode *defaultNode = megaApi->getNodeByPath(ui->eFolderPath->text().toUtf8().constData());
    if (defaultNode)
    {
        nodeSelector->setSelectedFolderHandle(defaultNode->getHandle());
        delete defaultNode;
    }

    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *node = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if (!node)
    {
        delete nodeSelector;
        return;
    }

    const char *nPath = megaApi->getNodePath(node);
    if (!nPath)
    {
        delete nodeSelector;
        delete node;
        return;
    }

    ui->eFolderPath->setText(QString::fromUtf8(nPath));
    delete nodeSelector;
    delete [] nPath;
    delete node;
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
    MegaNode *node = megaApi->getNodeByPath(ui->eFolderPath->text().toUtf8().constData());
    if (node && node->isFolder())
    {
        selectedHandle = node->getHandle();
        delete node;
        accept();
        return;
    }
    delete node;
    if (!ui->eFolderPath->text().compare(QString::fromUtf8("/MEGAsync Uploads")))
    {
        ui->bChange->setEnabled(false);
        ui->bOK->setEnabled(false);
        MegaNode *rootNode = megaApi->getRootNode();
        if (!rootNode)
        {
            return;
        }

        megaApi->createFolder(tr("MEGAsync Uploads").toUtf8().constData(), rootNode, delegateListener);
        delete rootNode;
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Folder not found: %1").arg(ui->eFolderPath->text()).toUtf8().constData());
    ui->eFolderPath->setText(QString::fromUtf8("/MEGAsync Uploads"));
    return;
}
