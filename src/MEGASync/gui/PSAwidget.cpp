#include "PSAwidget.h"
#include "ui_PSAwidgetwidget.h"
#include <QDesktopServices>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

PSAwidget::PSAwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSAwidget)
{
    ui->setupUi(this);
    ui->wImage->hide();

    hide();
}

PSAwidget::~PSAwidget()
{
    delete ui;
}

bool PSAwidget::setAnnounce(QString title, QString desc, QString urlMore, QImage image)
{
    if (title.isEmpty() || desc.isEmpty())
    {
        return false;
    }

    //FIXME. If title and desc are too long, need to elide
    ui->lTitle->setText(title);
    ui->lDesc->setText(desc);
    this->urlMore = urlMore;

    if (!image.isNull())
    {
        ui->bImage->setIcon(QPixmap::fromImage(image));
        ui->bImage->setIconSize(QSize(64, 64));
        ui->wImage->show();
    }

    show();
    return true;
}

void PSAwidget::removeAnnounce()
{
    ui->lTitle->setText(QString::fromUtf8(""));
    ui->lDesc->setText(QString::fromUtf8(""));
    ui->wImage->hide();

    hide();
}

void PSAwidget::on_bMore_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(urlMore));
}

void PSAwidget::on_bDismiss_clicked()
{
    hide();
}
