#include "ActiveTransfer.h"
#include "ui_ActiveTransfer.h"

ActiveTransfer::ActiveTransfer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveTransfer)
{
    ui->setupUi(this);
    fileName = "";
    ui->lType->setPixmap(QPixmap());
    ui->lPercentage->setText(QString());
    ui->pProgress->hide();
    ui->lType->hide();
}

ActiveTransfer::~ActiveTransfer()
{
    delete ui;
}

void ActiveTransfer::setFileName(QString fileName)
{
    this->fileName = fileName;
	QFont f = ui->lFileName->font();
	QFontMetrics fm = QFontMetrics(f);
	ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lFileName->width()));
}

void ActiveTransfer::setProgress(long long completedSize, long long totalSize)
{
	int permil = (1000*completedSize)/(totalSize+1);
	ui->pProgress->setProgress(permil);
	ui->lPercentage->setText(QString::number(permil/10) + "%");
    ui->pProgress->show();
    ui->lType->show();
}

void ActiveTransfer::setType(int type)
{
    this->type = type;
    if(type)
    {
		ui->lType->setPixmap(QPixmap("://images/tray_upload_ico.png"));
        ui->lPercentage->setStyleSheet("color: rgb(119, 186, 216);");
    }
    else
    {
		ui->lType->setPixmap(QPixmap("://images/tray_download_ico.png"));
        ui->lPercentage->setStyleSheet("color: rgb(122, 177, 72);");
	}
}

void ActiveTransfer::hideTransfer()
{
	ui->lFileName->setText(QString());
	ui->lType->setPixmap(QPixmap());
	ui->lPercentage->setText(QString());
	ui->pProgress->hide();
	ui->lType->hide();
}
