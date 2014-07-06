#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"
#include "gui/NodeSelector.h"
#include "control/Utilities.h"

UploadToMegaDialog::UploadToMegaDialog(MegaApi *megaApi, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UploadToMegaDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);
    this->megaApi = megaApi;
    this->delegateListener = new QTMegaRequestListener(megaApi, this);

    selectedHandle = mega::UNDEF;
    ui->eFolderPath->setText(tr("/MEGAsync Uploads"));
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

mega::handle UploadToMegaDialog::getSelectedHandle()
{
	return selectedHandle;
}

bool UploadToMegaDialog::isDefaultFolder()
{
	return ui->cDefaultPath->isChecked();
}

void UploadToMegaDialog::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    ui->bChange->setEnabled(true);
    ui->bOK->setEnabled(true);
    MegaNode *node = megaApi->getNodeByHandle(request->getNodeHandle());
	if(e->getErrorCode() != MegaError::API_OK || !node)
	{
        LOG(QString::fromAscii("ERROR: %1").arg(e->QgetErrorString()));
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
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, false, this);
	nodeSelector->nodesReady();
	int result = nodeSelector->exec();
	if(result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    mega::handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *node = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if(!node)
    {
        delete nodeSelector;
        return;
    }

    const char *nPath = megaApi->getNodePath(node);
    if(!nPath)
    {
        delete nodeSelector;
        delete node;
        return;
    }

    ui->eFolderPath->setText(QString::fromUtf8(nPath));
    delete nodeSelector;
    delete nPath;
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
    if(node && node->isFolder())
    {
        selectedHandle = node->getHandle();
        delete node;
        accept();
        return;
    }
    delete node;
    if(!ui->eFolderPath->text().compare(tr("/MEGAsync Uploads")))
    {
        ui->bChange->setEnabled(false);
        ui->bOK->setEnabled(false);
        MegaNode *rootNode = megaApi->getRootNode();
        megaApi->createFolder(tr("MEGAsync Uploads").toUtf8().constData(), rootNode, delegateListener);
        delete rootNode;
        return;
    }

    LOG("ERROR: FOLDER NOT FOUND");
    ui->eFolderPath->setText(tr("/MEGAsync Uploads"));
    return;
}
