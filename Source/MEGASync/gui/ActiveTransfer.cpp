#include "ActiveTransfer.h"
#include "ui_ActiveTransfer.h"
#include <QMouseEvent>

ActiveTransfer::ActiveTransfer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveTransfer)
{
    ui->setupUi(this);
    fileName = QString::fromAscii("");
    ui->lType->setText(QString());
    ui->lPercentage->setText(QString());
    ui->pProgress->hide();
    ui->lType->hide();
    regular = false;
    connect(ui->pProgress, SIGNAL(cancel(int,int)), this, SLOT(onCancelClicked(int,int)));
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

void ActiveTransfer::setProgress(long long completedSize, long long totalSize, bool cancellable)
{
    int permil = 0;
    if((totalSize>0) && (completedSize>0))
        permil = (1000*completedSize)/totalSize;
    regular = cancellable;
    ui->pProgress->setProgress(permil, cancellable);
    ui->lPercentage->setText(QString::number((permil+5)/10) + QString::fromAscii("%"));
    ui->pProgress->show();
    ui->lType->show();
    this->show();
}

void ActiveTransfer::setType(int type)
{
    this->type = type;
    QIcon icon;
    if(type)
    {
        icon.addFile(QString::fromUtf8(":/images/tray_upload_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->lPercentage->setStyleSheet(QString::fromAscii("color: rgb(119, 186, 216);"));
    }
    else
    {
        icon.addFile(QString::fromUtf8(":/images/tray_download_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->lPercentage->setStyleSheet(QString::fromAscii("color: rgb(122, 177, 72);"));
    }
#ifdef __APPLE__
        ui->lType->setIcon(icon);
        ui->lType->setIconSize(QSize(24, 24));
#else
        ui->lType->setPixmap(icon.pixmap(QSize(24, 24)));
#endif
}

void ActiveTransfer::hideTransfer()
{
    ui->lFileName->setText(QString::fromAscii(""));
    ui->lPercentage->setText(QString::fromAscii(""));
	ui->pProgress->hide();
    ui->lType->hide();
    this->hide();
}

void ActiveTransfer::mouseReleaseEvent(QMouseEvent *event)
{
    if(!regular || !(event->button()==Qt::RightButton)) return;
    emit cancel(event->x(), event->y());
}

void ActiveTransfer::onCancelClicked(int x, int y)
{
    emit cancel(ui->pProgress->pos().x()+x,
                 ui->pProgress->pos().y()+y);
}
