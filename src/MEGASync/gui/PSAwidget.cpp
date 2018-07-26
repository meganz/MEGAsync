#include "PSAwidget.h"
#include "ui_PSAwidget.h"
#include <QDesktopServices>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

PSAwidget::PSAwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSAwidget)
{
    ui->setupUi(this);

    minHeightAnimation = new QPropertyAnimation();
    maxHeightAnimation = new QPropertyAnimation();
    animationGroup = new QParallelAnimationGroup();
    animationGroup->addAnimation(minHeightAnimation);
    animationGroup->addAnimation(maxHeightAnimation);
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onAnimationFinished()));

    ui->pPSA->hide();
    ui->sWidget->hide();
    ui->wImage->hide();
}

PSAwidget::~PSAwidget()
{
    delete ui;
    delete minHeightAnimation;
    delete maxHeightAnimation;
    delete animationGroup;
}

bool PSAwidget::setAnnounce(QString title, QString desc, QString urlMore, QImage image)
{
    if (title.isEmpty() || desc.isEmpty())
    {
        return false;
    }

    QFont f = ui->lTitle->font();
    QFontMetrics fm = QFontMetrics(f);
    int width = ui->lTitle->width();
    ui->lTitle->setText(fm.elidedText(title, Qt::ElideRight, width));

    ui->lDesc->setFrameStyle(QFrame::Box);
    ui->lDesc->setText(desc);

    this->urlMore = urlMore;

    if (!image.isNull())
    {
        ui->bImage->setIcon(QPixmap::fromImage(image));
        ui->bImage->setIconSize(QSize(64, 64));
        ui->wImage->show();
    }   

    ui->pPSA->hide();
    ui->sWidget->show();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(0);
    maxHeightAnimation->setStartValue(0);
    minHeightAnimation->setEndValue(120);
    maxHeightAnimation->setEndValue(120);
    minHeightAnimation->setDuration(250);
    maxHeightAnimation->setDuration(250);
    animationGroup->start();

    return true;
}

void PSAwidget::removeAnnounce()
{
    ui->lTitle->setText(QString::fromUtf8(""));
    ui->lDesc->setText(QString::fromUtf8(""));
    ui->wImage->hide();

    ui->pPSA->hide();
    ui->sWidget->hide();
    setMinimumHeight(0);
    setMaximumHeight(0);
}

void PSAwidget::on_bMore_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(urlMore));
    emit moreclicked();
}

void PSAwidget::on_bDismiss_clicked()
{
    ui->pPSA->hide();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(120);
    maxHeightAnimation->setStartValue(120);
    minHeightAnimation->setEndValue(0);
    maxHeightAnimation->setEndValue(0);
    minHeightAnimation->setDuration(250);
    maxHeightAnimation->setDuration(250);
    animationGroup->start();

    emit dismissClicked();
}

void PSAwidget::onAnimationFinished()
{
    ui->pPSA->show();
}
