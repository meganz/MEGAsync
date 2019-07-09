#include "CircularUsageProgressBar.h"
#include <QPainter>
#include <QDebug>

CircularUsageProgressBar::CircularUsageProgressBar(QWidget *parent) :
    QWidget(parent), outerRadius(0), pbValue(0), penWidth(0)
{
    setPenColor(bkPen, QColor(QString::fromUtf8(DEFAULT_BKCOLOR)));
    setPenColor(fgPen, QColor(QString::fromUtf8(DEFAULT_FGCOLOR)));

    bkColor = QColor(QString::fromUtf8(DEFAULT_BKCOLOR));
    fgColor = QColor(QString::fromUtf8(DEFAULT_FGCOLOR));
    oqColor = QColor(QString::fromUtf8(DEFAULT_OQCOLOR));
    almostOqColor = QColor(QString::fromUtf8(DEFAULT_ALMOSTOQCOLOR));

    textValue = QString::fromUtf8("-");

    mark_warning.addFile(QString::fromUtf8(":/images/strong_mark.png"));
}

CircularUsageProgressBar::~CircularUsageProgressBar()
{

}

void CircularUsageProgressBar::paintEvent(QPaintEvent *event)
{
    double updatedOuterRadius = qMin(width(), height());
    if (updatedOuterRadius != outerRadius)
    {
        outerRadius = updatedOuterRadius;
        penWidth = outerRadius / 352.0 * 37;

        setPenColor(bkPen, bkColor, false);
        bkPen.setWidth(penWidth);

        setPenColor(fgPen, fgColor, false);
        fgPen.setWidth(penWidth);
    }

    QRectF baseRect(penWidth / 2, penWidth / 2, outerRadius - penWidth, outerRadius - penWidth);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);

    painter.fillRect(baseRect, Qt::NoBrush);

    // Draw background progress bar
    drawBackgroundBar(painter, baseRect);

    // Draw value arc
    double arcStep = 3.60 * pbValue;
    drawArcValue(painter, baseRect, arcStep);

    //Draw percentage text
    double innerRadius = outerRadius - penWidth / 2;
    double delta = (outerRadius - innerRadius) / 2;
    QRectF innerRect    = QRectF(delta, delta, innerRadius, innerRadius);
    drawText(painter, innerRect, innerRadius, pbValue);

    if (pbValue >= ALMOSTOVERQUOTA_VALUE) // If value higher than almost oq threshold show warning image
    {
        int pixWidth =  outerRadius / 44.0 * 19;
        int padding =  outerRadius / 44.0;
        painter.drawPixmap(outerRadius - pixWidth / 2 + padding, 0 + padding , pixWidth - 2 * padding, pixWidth - 2 * padding, mark_warning.pixmap(pixWidth - 2 * padding, pixWidth - 2));
    }
}

void CircularUsageProgressBar::drawBackgroundBar(QPainter &p, QRectF &baseRect)
{
    p.setPen(bkPen);
    p.setBrush(Qt::white);
    p.drawArc(baseRect, 90 * 16, -100 * 3.60 * 16); //360º
}

void CircularUsageProgressBar::drawArcValue(QPainter &p, const QRectF &baseRect, double arcLength)
{
    p.setPen(fgPen);
    p.setBrush(Qt::white);
    p.drawArc(baseRect, 90 * 16, -arcLength * 16);
}

void CircularUsageProgressBar::drawText(QPainter &p, const QRectF &innerRect, double innerRadius, double value)
{
    QFont f(font());
    f.setPixelSize(innerRadius * 0.33);
    f.setFamily(QString::fromUtf8("Source Sans Pro"));
    p.setFont(f);

    QRectF textRect(innerRect);
    p.setPen(!value ? bkColor : fgColor);
    p.drawText(textRect, Qt::AlignCenter, textValue);
}

void CircularUsageProgressBar::setPenColor(QPen &pen, QColor color, bool forceRepaint)
{
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);

    if (forceRepaint)
    {
        update();
    }
}

QColor CircularUsageProgressBar::getAlmostOqColor() const
{
    return almostOqColor;
}

void CircularUsageProgressBar::setAlmostOqColor(const QColor &value)
{
    almostOqColor = value;
    update();
}

QColor CircularUsageProgressBar::getOqColor() const
{
    return oqColor;
}

void CircularUsageProgressBar::setOqColor(const QColor &value)
{
    oqColor = value;
    update();
}

int CircularUsageProgressBar::getValue() const
{
    return pbValue;
}

void CircularUsageProgressBar::setValue(int value)
{
    if (pbValue != value)
    {
        if (value <= MIN_VALUE)
        {
            pbValue = MIN_VALUE;
            textValue = QString::fromUtf8("-");
            setPenColor(fgPen, fgColor, false);
        }
        else
        {
            textValue = QString::number(value).append(QString::fromUtf8("%"));
            pbValue = value;
            if (value >= MAX_VALUE)
            {
                fgColor = oqColor;
                setPenColor(fgPen, oqColor, false);
            }
            else if (value >= ALMOSTOVERQUOTA_VALUE)
            {
                fgColor = almostOqColor;
                setPenColor(fgPen, almostOqColor, false);
            }
            else
            {
                fgColor = QColor(QString::fromUtf8(DEFAULT_FGCOLOR));
                setPenColor(fgPen, fgColor, false);
            }
        }
        update();
    }
}

QColor CircularUsageProgressBar::getFgColor() const
{
    return fgColor;
}

void CircularUsageProgressBar::setFgColor(const QColor &color)
{
    fgColor = color;
    update();
}

QColor CircularUsageProgressBar::getBkColor() const
{
    return bkColor;
}

void CircularUsageProgressBar::setBkColor(const QColor &color)
{
    bkColor = color;
    update();
}
