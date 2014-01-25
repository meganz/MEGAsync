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
