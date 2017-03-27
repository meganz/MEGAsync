#include "DataUsageMenu.h"
#include <QPainter>

DataUsageMenu::DataUsageMenu(QWidget *parent) :
    QMenu(parent)
{
    setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; }"));
}

DataUsageMenu::~DataUsageMenu()
{

}

void DataUsageMenu::paintEvent(QPaintEvent *event)
{
    QVector<QPointF> points;
    float w = width()  - 0.5;
    float h = height() - 0.5;

    points << QPointF(0, 0)
           << QPointF(w, 0)
           << QPointF(w, h * 0.96)
           << QPointF(w * 0.53, h * 0.96)
           << QPointF(w * 0.50, h)
           << QPointF(w * 0.48, h * 0.96)
           << QPointF(0, h * 0.96);

    polygon = QPolygonF(points);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    painter.setPen(Qt::NoPen);

    QRegion maskRegion(polygon.toPolygon(), Qt::WindingFill);
    painter.drawPolygon(polygon);
    setMask(maskRegion);
}
