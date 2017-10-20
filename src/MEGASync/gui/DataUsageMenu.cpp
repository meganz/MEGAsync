#include "DataUsageMenu.h"
#include <QPainter>

DataUsageMenu::DataUsageMenu(QWidget *parent) :
    QMenu(parent)
{
#ifdef __APPLE__
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(QString::fromAscii("QMenu { background: transparent; padding-top: 8px; }"));
#else
    setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px; padding-right: 50px; } "));
#endif
}

DataUsageMenu::~DataUsageMenu()
{

}

void DataUsageMenu::paintEvent(QPaintEvent *event)
{
#ifdef __APPLE__
    QVector<QPointF> points;
    float w = width()  - 0.5;
    float h = height() - 0.5;

    points << QPointF(0, 0)
           << QPointF(w, 0)
           << QPointF(w, h * 0.96)
           << QPointF(w * 0.53, h * 0.96)
           << QPointF(w * 0.50, h)
           << QPointF(w * 0.47, h * 0.96)
           << QPointF(0, h * 0.96);

    polygon = QPolygonF(points);

    QImage imageMask(width(), height(), QImage::Format_ARGB32_Premultiplied);
    imageMask.fill(Qt::transparent);
    QPainter mask(&imageMask);
    mask.setRenderHints(QPainter::Antialiasing
                    | QPainter::SmoothPixmapTransform
                    | QPainter::HighQualityAntialiasing);
    mask.setPen(Qt::NoPen);
    mask.setBrush(Qt::white);
    mask.drawPolygon(polygon);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(QRect(0, 0, width(), height()), QPixmap::fromImage(imageMask));
#else
    QMenu::paintEvent(event);
#endif
}
