#ifndef IMAGE_DOWNLOADER_H
#define IMAGE_DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>
#include <QObject>
#include <QMap>

#include <memory>

struct ImageData
{
    QString url = QString();
    QSize size;
    QImage::Format format;

    ImageData(const QString& url,
              int width = 0,
              int height = 0,
              QImage::Format format = QImage::Format_ARGB32_Premultiplied)
        : url(url)
        , size(QSize(width, height))
        , format(format)
    {
    }
};

class ImageDownloader : public QObject
{
    Q_OBJECT

public:
    enum class Error
    {
        NoError = 0,
        NetworkError,
        InvalidUrl,
        EmptyData,
        InvalidImage
    };

    explicit ImageDownloader(QObject* parent = nullptr);
    explicit ImageDownloader(unsigned int timeout, QObject* parent = nullptr);
    virtual ~ImageDownloader() = default;

public slots:
    void downloadImage(const QString& imageUrl,
                       int width,
                       int height,
                       QImage::Format format = QImage::Format_ARGB32_Premultiplied);

signals:
    void downloadFinished(const QImage& image,
                          const QString& imageUrl);
    void downloadFinishedWithError(const QString& imageUrl,
                                   Error error,
                                   QNetworkReply::NetworkError networkError = QNetworkReply::NoError);

private slots:
    void onRequestImgFinished(QNetworkReply* reply);

private:
    QMap<QNetworkReply*, std::shared_ptr<ImageData>> mReplies;
    std::unique_ptr<QNetworkAccessManager> mManager;
    unsigned int mTimeout;

    bool validateReply(QNetworkReply* reply, const QString& url, QByteArray& bytes);
    void processImageData(const QByteArray& bytes, const std::shared_ptr<ImageData>& imageData);

};

#endif // IMAGE_DOWNLOADER_H

