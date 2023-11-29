#ifndef STANDARDICONPROVIDER_H
#define STANDARDICONPROVIDER_H

#include <QQuickImageProvider>

class StandardIconProvider : public QQuickImageProvider
{

public:
    StandardIconProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

};

#endif // STANDARDICONPROVIDER_H
