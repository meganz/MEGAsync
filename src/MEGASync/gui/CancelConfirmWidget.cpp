#include "CancelConfirmWidget.h"
#include "ui_CancelConfirmWidget.h"

#include "BlurredShadowEffect.h"
#include "Utilities.h"

CancelConfirmWidget::CancelConfirmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CancelConfirmWidget)
{
    ui->setupUi(this);
    setupAnimation();

    ui->pDismiss->setGraphicsEffect(CreateBlurredShadowEffect());
    ui->pProceed->setGraphicsEffect(CreateBlurredShadowEffect(QColor(217, 0, 7, 76)));

    connect(ui->pDismiss, &QPushButton::clicked, this, &CancelConfirmWidget::onDismissClicked);
    connect(ui->pProceed, &QPushButton::clicked, this, &CancelConfirmWidget::onProceedClicked);
}

CancelConfirmWidget::~CancelConfirmWidget()
{
    delete ui;
    delete mAnimation;
}

void CancelConfirmWidget::setStyleSheet(const QString &stylesheet)
{
    QWidget::setStyleSheet(stylesheet);
    for (int i=0; i<ui->cancelControlStack->count(); ++i)
    {
        ui->cancelControlStack->widget(i)->setStyleSheet(styleSheet());
    }
}

void CancelConfirmWidget::show()
{
    ui->cancelControlStack->setCurrentIndex(0);
    enableButtons(true);
}

void CancelConfirmWidget::setInCancellingStage()
{
    ui->lWaitAnimation->setMovie(mAnimation);
    mAnimation->start();
    ui->cancelControlStack->setCurrentIndex(1);
}

void CancelConfirmWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}

void CancelConfirmWidget::onDismissClicked()
{
    emit dismiss();
}

void CancelConfirmWidget::onProceedClicked()
{
    enableButtons(false);
    emit proceed();
}

void CancelConfirmWidget::setupAnimation()
{
    mAnimation = new QMovie(this);
    mAnimation->setCacheMode(QMovie::CacheAll);
    qreal ratio = Utilities::getDevicePixelRatio();
    QString gifFile = (ratio < 2) ? QString::fromUtf8(":/animations/cancelling.gif")
                                  : QString::fromUtf8(":/animations/cancelling@2x.gif");
    mAnimation->setFileName(gifFile);
}

void CancelConfirmWidget::enableButtons(bool value)
{
    ui->pDismiss->setEnabled(value);
    ui->pProceed->setEnabled(value);
}
