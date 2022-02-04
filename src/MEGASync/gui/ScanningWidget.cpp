#include "ScanningWidget.h"
#include "ui_ScanningWidget.h"

#include <QMovie>
#include "Utilities.h"

ScanningWidget::ScanningWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScanningWidget)
{
    ui->setupUi(this);

    movie = new QMovie(this);
    movie->setCacheMode(QMovie::CacheAll);

    ui->lScanning->setProperty("role", QString::fromLatin1("title"));
    ui->lExplanation->setProperty("role", QString::fromLatin1("details"));
}

ScanningWidget::~ScanningWidget()
{
    delete ui;
    delete movie;
}

void ScanningWidget::show()
{
    qreal ratio = Utilities::getDevicePixelRatio();
    QString gifFile = (ratio < 2) ? QString::fromUtf8(":/animations/scanning.gif")
                                  : QString::fromUtf8(":/animations/scanning@2x.gif");

    movie->setFileName(gifFile);
    if (movie->isValid())
    {
        ui->lAnimation->setMovie(movie);
        movie->start();
    }
}

void ScanningWidget::hide()
{
    movie->stop();
}

void ScanningWidget::on_pBlockingStageCancel_clicked()
{
    emit cancel();
}
