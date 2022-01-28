#include "ScanningWidget.h"
#include "ui_ScanningWidget.h"

#include <QMovie>

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
    movie->setFileName(QString::fromLatin1(":/animations/cat128.gif"));
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
