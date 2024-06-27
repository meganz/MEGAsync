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
    explicit ImageDownloader(QObject* parent = nullptr);
    virtual ~ImageDownloader() = default;

public slots:
    void downloadImage(const QString& imageUrl,
                       int width,
                       int height,
                       QImage::Format format = QImage::Format_ARGB32_Premultiplied);

signals:
    void downloadFinished(const QImage& image, const QString& imageUrl);

private slots:
    void onRequestImgFinished(QNetworkReply* reply);

private:
    std::unique_ptr<QNetworkAccessManager> mManager;
    QMap<QNetworkReply*, std::shared_ptr<ImageData>> mReplies;

};

#endif // IMAGE_DOWNLOADER_H

