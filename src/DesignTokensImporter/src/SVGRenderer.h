#ifndef SVGRENDERER_H
#define SVGRENDERER_H

#include <QPixmap>
#include <QByteArray>

class SVGRenderer
{
public:
    SVGRenderer() = default;

    QString getSVGImageData(const QString& name, const QSize& size, const QColor& color);
    QString getSVGImageData(const QString& name, const QSize& size, const QList<QColor> &colors);
    QPixmap getPixmapImage(const QString& name, const QSize& size, const QColor& color);
    QPixmap getPixmapImage(const QString& name, const QSize& size, const QList<QColor> &colors);

private:
    QPixmap renderSVGPixmap(const QByteArray& data, const QSize& size) const;
    QString renderSVGData(const QByteArray& data, const QSize& size) const;
    QByteArray colourSVG(const QByteArray& data, const QList<QColor> &colors) const;
    QByteArray colourSVG(const QByteArray& data, const QColor& color) const;
    bool checkImageCache(const QString& name);
    const QString& qrcPrefix();

    static QHash<QString, QByteArray> mImages;
};

#endif // SVGRENDERER_H
