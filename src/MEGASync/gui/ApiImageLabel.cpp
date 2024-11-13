#include "ApiImageLabel.h"

#include "Utilities.h"

#include <QPainter>

constexpr int DownloadTimeout = 5000;

ApiImageLabel::ApiImageLabel(QWidget* parent):
    QLabel(parent),
    mDownloader(std::make_unique<ImageDownloader>(DownloadTimeout))
{
    connect(mDownloader.get(),
            &ImageDownloader::downloadFinished,
            this,
            &ApiImageLabel::onDownloadFinished);
    connect(mDownloader.get(),
            &ImageDownloader::downloadFinishedWithError,
            this,
            &ApiImageLabel::onDownloadError);
}

void ApiImageLabel::setImageUrl(QString url)
{
    if (Utilities::getDevicePixelRatio() >= 2)
    {
        QString imageName = QFileInfo(url).fileName().split(QString::fromUtf8(".")).at(0);
        if (!imageName.contains(QRegExp(QString::fromUtf8("@2x$"))))
        {
            url.replace(imageName, imageName + QString::fromUtf8("@2x"));
        }
    }

    mDownloader->downloadImage(url);
}

void ApiImageLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!mImage.isNull())
    {
        if (mImage.size().width() < size().width() && mImage.size().height() < size().height())
        {
            QRect labelRect(QPoint(0, 0), size());
            QRect imageRect(mImage.rect());
            imageRect.moveCenter(labelRect.center());

            painter.drawImage(imageRect, mImage, mImage.rect());
        }
        else
        {
            painter.drawImage(QRect(QPoint(0, 0), size()), mImage, mImage.rect());
        }
    }
}

void ApiImageLabel::onDownloadFinished(const QImage& image, const QString&)
{
    mImage = image;
    emit imageReady(true);
}

void ApiImageLabel::onDownloadError(const QString&,
                                    ImageDownloader::Error,
                                    QNetworkReply::NetworkError)
{
    mImage = QImage();
    emit imageReady(false);
}

const QImage& ApiImageLabel::image() const
{
    return mImage;
}
