#include "SVGRenderer.h"

#include <QSvgRenderer>
#include <QPainter>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include <QLatin1String>
#include <QtDebug>

QHash<QString, QByteArray> SVGRenderer::mImages = QHash<QString, QByteArray>();
static const QLatin1String STOP_COLOR("stop-color");
static const QLatin1String FILL("fill");
static const QLatin1String STROKE("stroke");
static const QLatin1String OPACITY("opacity");

QPixmap SVGRenderer::getPixmapImage(const QString& name, const QSize& size, const QColor& color)
{
    if(checkImageCache(name))
    {
        QByteArray data = colourSVG(mImages[name], color);
        return renderSVGPixmap(data, size);
    }
    return QPixmap();
}

QPixmap SVGRenderer::getPixmapImage(const QString& name, const QSize& size, const QList<QColor>& colors)
{
    if(checkImageCache(name))
    {
        QByteArray data = colourSVG(mImages[name], colors);
        return renderSVGPixmap(data, size);
    }
    return QPixmap();
}

QString SVGRenderer::getSVGImageData(const QString &name, const QSize &size, const QColor& color)
{
    if(checkImageCache(name))
    {
        QByteArray data = colourSVG(mImages[name], color);
        return renderSVGData(data, size);
    }
    return QString();
}

QString SVGRenderer::getSVGImageData(const QString& name, const QSize& size, const QList<QColor>& colors)
{
    if(checkImageCache(name))
    {
        QByteArray data = colourSVG(mImages[name], colors);
        return renderSVGData(data, size);
    }
    return QString();
}

bool SVGRenderer::checkImageCache(const QString& name)
{
    if (!mImages.contains(name))
    {
       QString svgName(name);
       if(svgName.startsWith(qrcPrefix()))
       {
           svgName.remove(0,qrcPrefix().length());
       }

       QFile file(svgName);

       if (file.open(QIODevice::ReadOnly))
       {
          QByteArray data = file.readAll();
          mImages.insert(name, data);
       }
    }

    return mImages.contains(name);
}

QPixmap SVGRenderer::renderSVGPixmap(const QByteArray& data, const QSize& size) const
{
    qreal pixelRatio = QWindow().devicePixelRatio();
    auto scaledSize = QSize(qRound (pixelRatio * size.width()), qRound(pixelRatio * size.height()));

    QSvgRenderer svgRenderer(data);
    QPixmap image(scaledSize);
    QPainter painter;

    image.fill(Qt::transparent);

    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    svgRenderer.render(&painter);
    painter.end();

    image.setDevicePixelRatio(pixelRatio);

    return image;
}

QString SVGRenderer::renderSVGData(const QByteArray& data, const QSize& size) const
{
    QDomDocument doc;
    doc.setContent(data);

    QDomElement svgElement = doc.documentElement();
    svgElement.setAttribute("width", QString::number(size.width()));
    svgElement.setAttribute("height", QString::number(size.height()));

    // Convert the DOM tree back to a string
    QString modifiedSvgString;
    QTextStream stream(&modifiedSvgString);
    doc.save(stream, 4); // 4 is for indentation

    return modifiedSvgString;
}

QByteArray SVGRenderer::colourSVG(const QByteArray& data, const QList<QColor>& colors) const
{
    QDomDocument doc;
    doc.setContent(data);

   if (!colors.isEmpty())
   {       
       static QStringList elementsToPaint{QLatin1String("stop")};
       foreach (const QString& element, elementsToPaint)
       {
           QDomNodeList pathNodeList = doc.elementsByTagName(element);

           for (int nodeIndex  = 0; nodeIndex  < pathNodeList.count(); ++nodeIndex)
           {
               QDomNode node = pathNodeList.at(nodeIndex);
               if (node.isElement())
               {
                   auto pathElement = node.toElement();
                   auto colorName = colors.at(nodeIndex).name();

                   if(!pathElement.attribute(STOP_COLOR).isNull())
                   {
                       pathElement.setAttribute(STOP_COLOR, colorName.toUpper());
                   }

                   auto opacity = colors.at(nodeIndex).alphaF();
                   pathElement.setAttribute(QLatin1String("stop-opacity"), opacity);

                   pathElement = pathElement.nextSiblingElement(element);
               }
           }
       }
   }

   return doc.toByteArray();
}

QByteArray SVGRenderer::colourSVG(const QByteArray& data, const QColor& color) const
{
    QDomDocument doc;
    doc.setContent(data);

    static QStringList elementsToPaint{QLatin1String("circle"), QLatin1String("path"),
         QLatin1String("polygon"), QLatin1String("rect")};

    foreach (const QString& element, elementsToPaint)
    {
       QDomNodeList pathNodeList = doc.elementsByTagName(element);

       for (int nodeIndex = 0; nodeIndex < pathNodeList.count(); ++nodeIndex)
       {
          QDomNode node = pathNodeList.at(nodeIndex);
          if (node.isElement())
          {
             auto pathElement = node.toElement();

             if(!pathElement.attribute(FILL).isNull())
             {
                 pathElement.setAttribute(FILL, color.name());
             }
             if(!pathElement.attribute(STROKE).isNull())
             {
                 pathElement.setAttribute(STROKE, color.name());
             }

             pathElement.setAttribute(OPACITY, color.alphaF());

             pathElement = pathElement.nextSiblingElement(element);
          }
       }
    }

    return doc.toByteArray();
}


const QString& SVGRenderer::qrcPrefix() {
    static const QString prefix = QLatin1String("qrc");
    return prefix;
}

