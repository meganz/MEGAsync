#include "ImageDownloader.h"

#include "megaapi.h"

#include <QNetworkRequest>

namespace
{
constexpr int DefaultTimeout = 30000;
constexpr int StatusCodeOK = 200;
}

ImageDownloader::ImageDownloader(QObject* parent)
    : ImageDownloader(DefaultTimeout, parent)
{
}

ImageDownloader::ImageDownloader(unsigned int timeout, QObject* parent)
    : QObject(parent)
    , mManager(std::make_unique<QNetworkAccessManager>(nullptr))
    , mTimeout(timeout)
{
    connect(mManager.get(), &QNetworkAccessManager::finished,
            this, &ImageDownloader::onRequestImgFinished);
}

void ImageDownloader::downloadImage(const QString& imageUrl, QImage::Format format)
{
    QUrl url(imageUrl);
    if (!url.isValid())
    {
        return;
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(static_cast<int>(mTimeout));
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply* reply = mManager->get(request);
    if (reply)
    {
        auto imageData = std::make_shared<ImageData>(imageUrl, format);
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

    auto imageData = mReplies.take(reply);
    if (!imageData)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Received finished signal for unknown QNetworkReply");
        emit downloadFinishedWithError(QString(), Error::InvalidUrl);
        return;
    }

    QByteArray bytes;
    if (!validateReply(reply, imageData->url, bytes))
    {
        return;
    }

    processImageData(bytes, imageData);
}

bool ImageDownloader::validateReply(QNetworkReply* reply,
                                    const QString& url,
                                    QByteArray& bytes)
{
    bool success = true;
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCode.isValid() || (statusCode.toInt() != StatusCodeOK) || (reply->error() != QNetworkReply::NoError))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Error downloading image %s : %d",
                           reply->errorString().toUtf8().constData(), reply->error());
        emit downloadFinishedWithError(url, Error::NetworkError, reply->error());
        success = false;
    }

    if(success)
    {
        bytes = reply->readAll();
        if (bytes.isEmpty())
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               "Downloaded image data is empty");
            emit downloadFinishedWithError(url, Error::EmptyData, reply->error());
            success = false;
        }
    }

    return success;
}

void ImageDownloader::processImageData(const QByteArray& bytes,
                                       const std::shared_ptr<ImageData>& imageData)
{
    QImage image(QSize(), imageData->format);
    if (image.loadFromData(bytes))
    {
        emit downloadFinished(image, imageData->url);
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Failed to load image from downloaded data");
        emit downloadFinishedWithError(imageData->url, Error::InvalidImage, QNetworkReply::UnknownContentError);
    }
}
