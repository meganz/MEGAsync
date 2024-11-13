#ifndef APIIMAGELABEL_H
#define APIIMAGELABEL_H

#include "ImageDownloader.h"

#include <QLabel>

class ApiImageLabel: public QLabel
{
    Q_OBJECT

public:
    ApiImageLabel(QWidget* parent);

    void setImageUrl(QString url);
    const QImage& image() const;

protected:
    void paintEvent(QPaintEvent*) override;

signals:
    void imageReady(bool isValid);

private slots:
    void onDownloadFinished(const QImage& image, const QString& imageUrl);
    void onDownloadError(const QString&, ImageDownloader::Error, QNetworkReply::NetworkError);

private:
    std::unique_ptr<ImageDownloader> mDownloader;
    QImage mImage;
};

#endif // APIIMAGELABEL_H
