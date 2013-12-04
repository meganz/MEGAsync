#include "UploadToMegaDialog.h"
#include "ui_UploadToMegaDialog.h"

UploadToMegaDialog::UploadToMegaDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UploadToMegaDialog)
{
	ui->setupUi(this);
}

UploadToMegaDialog::~UploadToMegaDialog()
{
	delete ui;
}
