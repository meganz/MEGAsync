#include "CircularUsageProgressBar.h"
#include <QPainter>
#include <QDebug>
#include <math.h>

CircularUsageProgressBar::CircularUsageProgressBar(QWidget *parent) :
    QWidget(parent), outerRadius(0), penWidth(0)
{
    setPenColor(bkPen, QColor(QString::fromUtf8(DEFAULT_BKCOLOR)));
    setPenColor(fgPen, QColor(QString::fromUtf8(DEFAULT_FGCOLOR)));

    bkColor = QColor(QString::fromUtf8(DEFAULT_BKCOLOR));
    fgColor = QColor(QString::fromUtf8(DEFAULT_FGCOLOR));
    oqColor = QColor(QString::fromUtf8(DEFAULT_OQCOLOR));
    almostOqColor = QColor(QString::fromUtf8(DEFAULT_ALMOSTOQCOLOR));

    currentColor = fgColor;
    textValue = QString::fromUtf8("\xe2\x80\x94");

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

        setPenColor(fgPen, currentColor, false);
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
    qreal factor_decrease = 0.86;
    qreal factor = 1.0;
    auto aux = value;
    while (aux >= 1000)
    {
        factor *= factor_decrease;
        aux = aux / 10;
    }
    int pixelSize = innerRadius * 0.30;
    f.setPixelSize( std::max(5.0, floor(pixelSize * factor)) );
    f.setFamily(QString::fromUtf8("Lato"));
    p.setFont(f);

    QRectF textRect(innerRect);
    p.setPen(currentColor);
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

void CircularUsageProgressBar::setAlmostOqColor(const QColor &color)
{
    almostOqColor = color;
    update();
}

QColor CircularUsageProgressBar::getOqColor() const
{
    return oqColor;
}

void CircularUsageProgressBar::setOqColor(const QColor &color)
{
    oqColor = color;
    update();
}

int CircularUsageProgressBar::getValue() const
{
    return pbValue;
}

void CircularUsageProgressBar::setValue(int value, bool unknownTotal)
{
    if (value < CircularUsageProgressBar::MINVALUE)
    {
        value = CircularUsageProgressBar::MINVALUE;
    }
    if (pbValue != value || pbValue == -1
            || (textValue == QString::fromUtf8("\xe2\x80\x94") && !unknownTotal)
            || (textValue != QString::fromUtf8("\xe2\x80\x94") && unknownTotal)
            )
    {
        if (unknownTotal)
        {
            textValue = QString::fromUtf8("\xe2\x80\x94");
        }
        else
        {
            textValue = tr("[A]%").replace(QStringLiteral("[A]"), QString::number(value));
        }
        pbValue = value;
        if (value >= CircularUsageProgressBar::MAXVALUE)
        {
            currentColor = oqColor;
            setPenColor(fgPen, oqColor, false);
        }
        else if (value >= ALMOSTOVERQUOTA_VALUE)
        {
            currentColor = almostOqColor;
            setPenColor(fgPen, almostOqColor, false);
        }
        else
        {
            currentColor = fgColor;
            setPenColor(fgPen, currentColor, false);
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
