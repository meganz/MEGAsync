#include "CircularUsageProgressBar.h"
#include <QPainter>
#include <QDebug>
#include <math.h>

static const QString hyphenUtf8Code{QString::fromUtf8("\xe2\x80\x94")};

CircularUsageProgressBar::CircularUsageProgressBar(QWidget *parent) :
    QWidget(parent), penWidth(0), outerRadius(0)
{
    setPenColor(bkPen, QColor(QString::fromUtf8(DEFAULT_BKCOLOR)));
    setPenColor(fgPen, QColor(QString::fromUtf8(DEFAULT_FGCOLOR)));

    bkColor = QColor(QString::fromUtf8(DEFAULT_BKCOLOR));
    fgColor = QColor(QString::fromUtf8(DEFAULT_FGCOLOR));
    oqColor = QColor(QString::fromUtf8(DEFAULT_OQCOLOR));
    almostOqColor = QColor(QString::fromUtf8(DEFAULT_ALMOSTOQCOLOR));

    currentColor = fgColor;
    textValue = hyphenUtf8Code;

    markFull.addFile(QString::fromUtf8(":/images/icon_error.png"));
    markWarning.addFile(QString::fromUtf8(":/images/icon_warning.png"));
}

void CircularUsageProgressBar::paintEvent(QPaintEvent*)
{
    double updatedOuterRadius = qMin(width(), height());
    if (updatedOuterRadius != outerRadius)
    {
        outerRadius = updatedOuterRadius;
        penWidth = outerRadius / 352.0 * 37;

        setPenColor(bkPen, bkColor, false);
        bkPen.setWidth(static_cast<int>(penWidth));

        setPenColor(fgPen, currentColor, false);
        fgPen.setWidth(static_cast<int>(penWidth));
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
    double arcStep = 3.60 * progressBarValue;
    drawArcValue(painter, baseRect, arcStep);

    //Draw percentage text
    double innerRadius = outerRadius - penWidth / 2;
    double delta = (outerRadius - innerRadius) / 2;
    QRectF innerRect    = QRectF(delta, delta, innerRadius, innerRadius);
    drawText(painter, innerRect, innerRadius, progressBarValue);

    if (progressBarValue >= ALMOSTOVERQUOTA_VALUE) // If value higher than almost oq threshold show warning image
    {
        const auto icon{progressBarValue >= CircularUsageProgressBar::MAXVALUE ? markFull : markWarning};
        int pixWidth =  outerRadius / 44.0 * 19;
        int padding =  outerRadius / 44.0;
        painter.drawPixmap(outerRadius - pixWidth / 2 + padding, 0 + padding , pixWidth - 2 * padding,
                           pixWidth - 2 * padding, icon.pixmap(pixWidth - 2 * padding, pixWidth - 2));
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
    int pixelSize = innerRadius * 0.33;
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
    return progressBarValue;
}

void CircularUsageProgressBar::setValue(int value)
{
    if (value < CircularUsageProgressBar::MINVALUE)
    {
        value = CircularUsageProgressBar::MINVALUE;
    }

    if (progressBarValue != value)
    {
        textValue = QString::number(value).append(QString::fromUtf8("%"));
        progressBarValue = value;
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

void CircularUsageProgressBar::setEmptyBarTotalValueUnknown()
{
    textValue = hyphenUtf8Code;
    progressBarValue = 0;
    currentColor = fgColor;
    setPenColor(fgPen, currentColor, false);
    update();

}

void CircularUsageProgressBar::setFullBarTotalValueUnkown()
{
    textValue = hyphenUtf8Code;
    progressBarValue = CircularUsageProgressBar::MAXVALUE;
    currentColor = oqColor;
    setPenColor(fgPen, currentColor, false);
    update();
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
