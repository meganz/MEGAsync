#include "RecentFile.h"
#include "ui_RecentFile.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "platform/Platform.h"

#include <QImageReader>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

RecentFile::RecentFile(QWidget *parent) :
    TransferItem(parent),
    ui(new Ui::RecentFile)
{
    ui->setupUi(this);

    getLinkButtonEnabled = false;

    ui->lGetLink->installEventFilter(this);
    update();
}

RecentFile::~RecentFile()
{
    delete ui;
}

void RecentFile::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);

    QFont f = ui->lFileName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lFileName->width()));
    ui->lFileName->setToolTip(fileName);

    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapSmall(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(20, 22));
}

void RecentFile::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon;

    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
            icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;
        default:
            break;
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
}

void RecentFile::setTransferState(int value)
{
    TransferItem::setTransferState(value);
//    switch (transferState)
//    {
//    case MegaTransfer::STATE_COMPLETED:
//    case MegaTransfer::STATE_FAILED:
//        finishTransfer();
//        break;
//    case MegaTransfer::STATE_CANCELLED:
//    default:
//        break;
//    }
}

bool RecentFile::getLinkButtonClicked(QPoint pos)
{
    if (!getLinkButtonEnabled)
    {
        return false;
    }

    switch (transferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
        if (ui->lGetLink->rect().contains(ui->lGetLink->mapFrom(this, pos)))
        {
            return true;
        }
        break;
    case MegaTransfer::STATE_CANCELLED:
    default:
        break;
    }

    return false;
}

void RecentFile::mouseHoverTransfer(bool isHover)
{
    if (isHover)
    {
        getLinkButtonEnabled = true;
        ui->lGetLink->removeEventFilter(this);
        ui->lGetLink->update();
    }
    else
    {
        getLinkButtonEnabled = false;
        ui->lGetLink->installEventFilter(this);
        ui->lGetLink->update();
    }

    emit refreshTransfer(this->getTransferTag());
}

bool RecentFile::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

QSize RecentFile::minimumSizeHint() const
{
    return QSize(400, 32);
}
QSize RecentFile::sizeHint() const
{
    return QSize(400, 32);
}

