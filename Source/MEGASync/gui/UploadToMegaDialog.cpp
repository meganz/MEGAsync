#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"
#include "gui/NodeSelector.h"
#include "control/Utilities.h"

UploadToMegaDialog::UploadToMegaDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UploadToMegaDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);
	this->delegateListener = new QTMegaRequestListener(this);
}

UploadToMegaDialog::~UploadToMegaDialog()
{
    delete ui;
}

void UploadToMegaDialog::initialize(MegaApi *megaApi)
{
    selectedHandle = UNDEF;
    ui->eFolderPath->setText(tr("/MEGAsync Uploads"));
    ui->cDefaultPath->setChecked(false);
    this->megaApi = megaApi;
    ui->bChange->setEnabled(true);
    ui->bOK->setEnabled(true);
}

handle UploadToMegaDialog::getSelectedHandle()
{
	return selectedHandle;
}

bool UploadToMegaDialog::isDefaultFolder()
{
	return ui->cDefaultPath->isChecked();
}

void UploadToMegaDialog::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
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
		return;

    mega::handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *node = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    const char *nPath = megaApi->getNodePath(node);
    ui->eFolderPath->setText(QString::fromUtf8(nPath));
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
    if(node && node->getType()!=FILENODE)
    {
        selectedHandle = node->getHandle();
        delete node;
        accept();
        return;
    }

    if(!ui->eFolderPath->text().compare(tr("/MEGAsync Uploads")))
    {
        ui->bChange->setEnabled(false);
        ui->bOK->setEnabled(false);
        MegaNode *rootNode = megaApi->getRootNode();
        megaApi->createFolder(tr("MEGAsync Uploads").toUtf8().constData(), rootNode, delegateListener);
        delete rootNode;
        delete node;
        return;
    }

    LOG("ERROR: FOLDER NOT FOUND");
    ui->eFolderPath->setText(tr("/MEGAsync Uploads"));
    delete node;
    return;
}
