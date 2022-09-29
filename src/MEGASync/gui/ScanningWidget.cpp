#include "ScanningWidget.h"
#include "ui_ScanningWidget.h"

#include <QMovie>
#include "BlurredShadowEffect.h"
#include "Utilities.h"

ScanningWidget::ScanningWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::ScanningWidget)
{
    mUi->setupUi(this);
    mMovie = new QMovie(this);
    mMovie->setCacheMode(QMovie::CacheAll);

    qreal ratio = Utilities::getDevicePixelRatio();
    QString gifFile = (ratio < 2) ? QString::fromUtf8(":/animations/scanning.gif")
                                  : QString::fromUtf8(":/animations/scanning@2x.gif");
    mMovie->setFileName(gifFile);

    mUi->lScanning->setProperty("role", QString::fromLatin1("title"));
    mUi->lExplanation->setProperty("role", QString::fromLatin1("details"));

    mUi->pBlockingStageCancel->setGraphicsEffect(CreateBlurredShadowEffect());
    connect(mUi->pBlockingStageCancel, &QPushButton::clicked,
            this, &ScanningWidget::onCancelClicked);
}

ScanningWidget::~ScanningWidget()
{
    delete mUi;
    delete mMovie;
}

void ScanningWidget::show()
{
    startAnimation();

    mUi->pBlockingStageCancel->show();
    mUi->pBlockingStageCancel->setEnabled(true);
}

void ScanningWidget::hide()
{
    mMovie->stop();
    mUi->lAnimation->setMovie(nullptr);
}

void ScanningWidget::disableCancelButton()
{
    mUi->pBlockingStageCancel->setEnabled(false);
}

void ScanningWidget::updateAnimation()
{
    if(mMovie->state() == QMovie::Running)
    {
        mMovie->stop();
        mUi->lAnimation->setMovie(nullptr);
    }
    startAnimation();
}

void ScanningWidget::onCancelClicked()
{
    emit cancel();
}

void ScanningWidget::startAnimation()
{
    if (mMovie->isValid())
    {
        mUi->lAnimation->setMovie(mMovie);
        mMovie->start();
    }
}
