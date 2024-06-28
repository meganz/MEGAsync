#include "PSAwidget.h"

#include "ui_PSAwidget.h"

#include <QDesktopServices>
#include <Utilities.h>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

namespace
{
constexpr int DefaultSize = 64;
constexpr int DownloadTimeout = 5000;
}

PSAwidget::PSAwidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PSAwidget)
    , mDownloader(std::make_unique<ImageDownloader>(DownloadTimeout, this))
{
    ui->setupUi(this);

    this->ready = false;
    this->shown = false;

    connect(mDownloader.get(), &ImageDownloader::downloadFinished,
            this, &PSAwidget::onDownloadFinished);
    connect(mDownloader.get(), &ImageDownloader::downloadFinishedWithError,
            this, &PSAwidget::onDownloadError);

    minHeightAnimation = new QPropertyAnimation();
    maxHeightAnimation = new QPropertyAnimation();
    animationGroup = new QParallelAnimationGroup();
    animationGroup->addAnimation(minHeightAnimation);
    animationGroup->addAnimation(maxHeightAnimation);
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onAnimationFinished()));

    ui->pPSA->hide();
    ui->sWidget->hide();
    ui->wImage->hide();
    ui->bMore->hide();
}

PSAwidget::~PSAwidget()
{
    delete animationGroup;
    delete ui;
}

void PSAwidget::setAnnounce(int id, QString title, QString desc, QString urlImage, QString textButton, QString urlClick)
{
    removeAnnounce();

    info.idPSA = id;
    info.title = title;
    info.desc = desc;
    info.urlImage = urlImage;
    info.textButton = textButton;
    info.urlClick = urlClick;

    if (Utilities::getDevicePixelRatio() >= 2)
    {
        QString imageName = QFileInfo(urlImage).fileName().split(QString::fromUtf8(".")).at(0);
        if (!imageName.contains(QRegExp(QString::fromUtf8("@2x$"))))
        {
            urlImage.replace(imageName, imageName + QString::fromUtf8("@2x"));
        }
    }

    mDownloader->downloadImage(urlImage, DefaultSize, DefaultSize);
}

bool PSAwidget::isPSAready()
{
    return ready;
}

bool PSAwidget::isPSAshown()
{
    return shown;
}

void PSAwidget::setPSAImage(QImage image)
{
    ui->lTitle->ensurePolished();
    int width = ui->lTitle->width();
    ui->lTitle->setText(ui->lTitle->fontMetrics().elidedText(info.title, Qt::ElideRight, width));

    ui->lDesc->setFrameStyle(QFrame::Box);
    ui->lDesc->setText(info.desc);

    if (!info.textButton.isEmpty())
    {
        ui->bMore->setText(info.textButton);
        ui->bMore->show();
    }

    if (!image.isNull())
    {
        ui->bImage->setIcon(QPixmap::fromImage(image));
        ui->bImage->setIconSize(QSize(DefaultSize, DefaultSize));
        ui->wImage->show();
    }

    ready = true;
}

void PSAwidget::showPSA()
{
    if (shown || !ready)
    {
        return;
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
    shown = true;
}

PSA_info PSAwidget::getPSAdata()
{
    return info;
}

void PSAwidget::hidePSA(bool animated)
{
    if (!shown)
    {
        return;
    }

    shown = false;
    ui->pPSA->hide();
    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(120);
    maxHeightAnimation->setStartValue(120);
    minHeightAnimation->setEndValue(0);
    maxHeightAnimation->setEndValue(0);
    minHeightAnimation->setDuration(animated ? 250 : 1);
    maxHeightAnimation->setDuration(animated ? 250 : 1);
    animationGroup->start();
}

void PSAwidget::removeAnnounce()
{
    info.clear();

    ui->bImage->setIcon(QIcon());
    ui->lTitle->setText(QString::fromUtf8(""));
    ui->lDesc->setText(QString::fromUtf8(""));
    ui->bMore->setText(QString::fromUtf8(""));
    ui->bMore->hide();
    ui->wImage->hide();
    ui->sWidget->hide();
    ui->pPSA->hide();

    if (shown)
    {
        hidePSA();
    }
    ready = false;
}

void PSAwidget::on_bMore_clicked()
{
    Utilities::openUrl(QUrl(info.urlClick));
    on_bDismiss_clicked();
}

void PSAwidget::on_bDismiss_clicked()
{
    hidePSA(true);
    emit PSAseen(info.idPSA);
    removeAnnounce();
}

void PSAwidget::onAnimationFinished()
{
    ui->pPSA->setVisible(shown);
}

void PSAwidget::onDownloadFinished(const QImage& image,
                                   const QString& imageUrl)
{
    Q_UNUSED(imageUrl);
    setPSAImage(image);
}

void PSAwidget::onDownloadError(const QString& imageUrl,
                                ImageDownloader::Error error,
                                QNetworkReply::NetworkError networkError)
{
    Q_UNUSED(imageUrl);
    Q_UNUSED(networkError);

    if(error == ImageDownloader::Error::NetworkError)
    {
        setPSAImage();
    }
}
