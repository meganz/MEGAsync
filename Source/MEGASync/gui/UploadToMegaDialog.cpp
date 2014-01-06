#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"
#include "gui/NodeSelector.h"

UploadToMegaDialog::UploadToMegaDialog(MegaApi *megaApi, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UploadToMegaDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);

	selectedHandle = UNDEF;
    ui->eFolderPath->setText(tr("/MEGAsync Uploads"));
	this->megaApi = megaApi;
	this->delegateListener = new QTMegaRequestListener(this);
}

UploadToMegaDialog::~UploadToMegaDialog()
{
	delete ui;
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
	Node *node = megaApi->getNodeByHandle(request->getNodeHandle());
	if(e->getErrorCode() != MegaError::API_OK || !node)
	{
		cout << "ERROR: " << e->getErrorString() << endl;
		this->reject();
		return;
	}

	selectedHandle = node->nodehandle;
	accept();
}

void UploadToMegaDialog::on_bChange_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, false, this);
	nodeSelector->nodesReady();
	int result = nodeSelector->exec();

	if(result != QDialog::Accepted)
		return;

	handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    ui->eFolderPath->setText(QString::fromUtf8(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle))));
}

void UploadToMegaDialog::on_buttonBox_accepted()
{
	Node *node = megaApi->getNodeByPath(ui->eFolderPath->text().toUtf8().constData());
	if(node && node->type!=FILENODE)
	{
		selectedHandle = node->nodehandle;
		cout << "Node valid. Accepting dialog" << endl;
		accept();
		return;
	}

    if(!ui->eFolderPath->text().compare(tr("/MEGAsync Uploads")))
	{
        megaApi->createFolder(tr("MEGAsync Uploads").toUtf8().constData(), megaApi->getRootNode(), delegateListener);
		return;
	}

	cout << "ERROR: FOLDER NOT FOUND" << endl;
    ui->eFolderPath->setText(tr("/MEGAsync Uploads"));
	return;
}
