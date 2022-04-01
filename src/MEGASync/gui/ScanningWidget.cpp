#include "ScanningWidget.h"
#include "ui_ScanningWidget.h"

#include <QMovie>
#include "Utilities.h"

ScanningWidget::ScanningWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::ScanningWidget)
{
    mUi->setupUi(this);
    mMovie = new QMovie(this);
    mMovie->setCacheMode(QMovie::CacheAll);

    mUi->lScanning->setProperty("role", QString::fromLatin1("title"));
    mUi->lExplanation->setProperty("role", QString::fromLatin1("details"));
}

ScanningWidget::~ScanningWidget()
{
    delete mUi;
    delete mMovie;
}

void ScanningWidget::show()
{
    qreal ratio = Utilities::getDevicePixelRatio();
    QString gifFile = (ratio < 2) ? QString::fromUtf8(":/animations/scanning.gif")
                                  : QString::fromUtf8(":/animations/scanning@2x.gif");

    mMovie->setFileName(gifFile);
    if (mMovie->isValid())
    {
        mUi->lAnimation->setMovie(mMovie);
        mMovie->start();
    }
}

void ScanningWidget::hide()
{
    mMovie->stop();
    mUi->lAnimation->setMovie(nullptr);
}

void ScanningWidget::on_pBlockingStageCancel_clicked()
{
    emit cancel();
}
