#include "ImageDownloader.h"

#include "megaapi.h"

#include <QNetworkRequest>

ImageDownloader::ImageDownloader(QObject* parent)
    : QObject(parent)
    , mManager(new QNetworkAccessManager(this))
{
    connect(mManager.get(), &QNetworkAccessManager::finished,
            this, &ImageDownloader::onRequestImgFinished);
}

void ImageDownloader::downloadImage(const QString& imageUrl,
                                    int width,
                                    int height,
                                    QImage::Format format)
{
    QUrl url(imageUrl);
    if (!url.isValid())
    {
        return;
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply* reply = mManager->get(request);
    if (reply)
    {
        auto imageData = std::make_shared<ImageData>(imageUrl, width, height, format);
        mReplies.insert(reply, imageData);
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Failed to create QNetworkReply for URL: %s", imageUrl.toUtf8().constData());
    }
}


void ImageDownloader::onRequestImgFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (!mReplies.contains(reply))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Received finished signal for unknown QNetworkReply");
        return;
    }

    auto imageData = mReplies.take(reply);
    if (reply->error())
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Error downloading image: %s", reply->errorString().toUtf8().constData());
        return;
    }

    QByteArray bytes = reply->readAll();
    if (bytes.isEmpty())
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Downloaded image data is empty");
        return;
    }

    QImage image(imageData->size, imageData->format);
    if (!image.loadFromData(bytes))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Failed to load image from downloaded data");
        return;
    }

    emit downloadFinished(image, imageData->url);
}
