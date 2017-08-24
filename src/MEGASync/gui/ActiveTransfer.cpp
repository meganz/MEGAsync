#include "ActiveTransfer.h"
#include "ui_ActiveTransfer.h"
#include "control/Utilities.h"
#include <QMouseEvent>

ActiveTransfer::ActiveTransfer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveTransfer)
{
    ui->setupUi(this);
    fileName = QString::fromAscii("");
    ui->pProgress->hide();
    ui->lTransferType->hide();
    ui->lFileType->hide();
    regular = false;
    active = false;

    ui->lFileName->setTextInteractionFlags(Qt::NoTextInteraction);
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
    ui->lFileName->setStyleSheet(QString::fromUtf8("color: grey;"));
    ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lFileName->width()));

    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapSmall(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(20, 22));
}

void ActiveTransfer::setProgress(long long completedSize, long long totalSize, bool cancellable)
{
    unsigned int permil = 0;

    if ((totalSize > 0) && (completedSize > 0))
    {
        permil = (1000 * completedSize) / totalSize;
    }

    if (permil > 1000)
    {
        permil = 1000;
    }

    active = true;
    regular = cancellable;
    ui->pProgress->setValue(permil);
    ui->pProgress->show();
    ui->lTransferType->show();
    ui->lFileType->show();
    show();
}

void ActiveTransfer::setType(int type)
{
    this->type = type;
    QIcon icon;
    if (type)
    {
        icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->pProgress->setStyleSheet(QString::fromUtf8("QProgressBar#pProgress{background-color: #ececec;}"
                                                        "QProgressBar#pProgress::chunk {background-color: #2ba6de;}"));

    }
    else
    {
        icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->pProgress->setStyleSheet(QString::fromUtf8("QProgressBar#pProgress{background-color: #ececec;}"
                                                        "QProgressBar#pProgress::chunk {background-color: #31b500;}"));
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
}

void ActiveTransfer::hideTransfer()
{
    active = false;
    ui->lFileName->setText(QString::fromAscii(""));
    ui->pProgress->hide();
    ui->lTransferType->hide();
    ui->lFileType->hide();
    hide();
}

bool ActiveTransfer::isActive()
{
    return active;
}

void ActiveTransfer::mouseReleaseEvent(QMouseEvent *event)
{
    if (!regular || !(event->button() == Qt::RightButton))
    {
        return;
    }

    emit cancel(event->x(), event->y());
}
