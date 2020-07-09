#include "CircularUsageProgressBar.h"
#include <QPainter>
#include <QDebug>
#include <math.h>

CircularUsageProgressBar::CircularUsageProgressBar(QWidget *parent) :
    QWidget(parent), penWidth(0), outerRadius(0),
    markWarning{QStringLiteral(":/images/icon_warning_24.png")},
    markFull{QStringLiteral(":/images/icon_error_24.png")},
    dynamicTransferBlue{QStringLiteral(":/images/dynamic_transfer_icon_32.png")},
    dynamicTransferRed{QStringLiteral(":/images/dynamic_transfer_overquota_icon.png")},
    totalValueUnkown{true}
{
    setPenColor(backgroundPen, QColor(QString::fromUtf8(DEFAULT_BKCOLOR)));
    setPenColor(foregroundPen, QColor(QString::fromUtf8(DEFAULT_FGCOLOR)));

    backgroundColor = QColor(QString::fromUtf8(DEFAULT_BKCOLOR));
    foregroundColor = QColor(QString::fromUtf8(DEFAULT_FGCOLOR));
    overquotaColor = QColor(QString::fromUtf8(DEFAULT_OQCOLOR));
    almostOverquotaColor = QColor(QString::fromUtf8(DEFAULT_ALMOSTOQCOLOR));

    currentColor = foregroundColor;
}

void CircularUsageProgressBar::paintEvent(QPaintEvent*)
{
    constexpr auto padingPixels{6};
    double updatedOuterRadius = qMin(width(), height()) - padingPixels;
    if (updatedOuterRadius != outerRadius)
    {
        outerRadius = updatedOuterRadius;
        penWidth = outerRadius / 352.0 * 37;

        setPenColor(backgroundPen, backgroundColor, false);
        backgroundPen.setWidth(static_cast<int>(penWidth));

        setPenColor(foregroundPen, currentColor, false);
        foregroundPen.setWidth(static_cast<int>(penWidth));
    }

    QRectF baseRect(penWidth / 2, (penWidth / 2) + padingPixels / 2, outerRadius - penWidth, outerRadius - penWidth);
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

    //Draw percentage text or dynamic transfer icon
    const auto innerRadius{outerRadius - penWidth / 2};
    const auto delta{(outerRadius - innerRadius) / 2};
    const auto innerRect{QRectF(delta, delta + padingPixels / 2, innerRadius, innerRadius)};
    if(totalValueUnkown)
    {
        constexpr auto dynamicIconNativeSizePixels{QSize{36, 36}};
        auto dynamicIconRect{QRectF(QPoint(0, 0), dynamicIconNativeSizePixels)};
        dynamicIconRect.moveCenter(innerRect.center());
        QIcon dynamicQuotaIcon{dynamicTransferBlue};
        if(progressBarValue == CircularUsageProgressBar::MAXVALUE)
        {
            dynamicQuotaIcon = dynamicTransferRed;
        }
        painter.drawPixmap(dynamicIconRect.toRect(), dynamicQuotaIcon.pixmap(dynamicIconNativeSizePixels));
    }
    else
    {
        drawText(painter, innerRect, innerRadius, progressBarValue);
    }

    if (progressBarValue >= ALMOSTOVERQUOTA_VALUE) // If value higher than almost oq threshold show warning image
    {
        constexpr auto nativeOuterRadius{44.0};
        const auto ratio{outerRadius / nativeOuterRadius};
        constexpr auto iconSizePixels{24};
        const auto pixmapTotalSideLength{ratio * iconSizePixels};
        constexpr auto iconPaddingX{3};
        const auto x{outerRadius - (pixmapTotalSideLength / 2) - iconPaddingX};
        constexpr auto iconPaddingY{5};
        const auto y{padingPixels / 2 - iconPaddingY};
        const auto width{pixmapTotalSideLength};
        const auto height{pixmapTotalSideLength};
        const auto icon{progressBarValue >= CircularUsageProgressBar::MAXVALUE ? markFull : markWarning};
        const auto pixmap{icon.pixmap(iconSizePixels, iconSizePixels)};
        painter.drawPixmap(x, y, width, height, pixmap);
    }
}

void CircularUsageProgressBar::drawBackgroundBar(QPainter &p, QRectF &baseRect)
{
    p.setPen(backgroundPen);
    p.setBrush(Qt::white);
    p.drawArc(baseRect, 90 * 16, -100 * 3.60 * 16); //360º
}

void CircularUsageProgressBar::drawArcValue(QPainter &p, const QRectF &baseRect, double arcLength)
{
    p.setPen(foregroundPen);
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
    const auto penColor(value < ALMOSTOVERQUOTA_VALUE ? DEFAULT_TEXT_COLOR : currentColor);
    p.setPen(penColor);
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

QColor CircularUsageProgressBar::getAlmostOverquotaColor() const
{
    return almostOverquotaColor;
}

void CircularUsageProgressBar::setAlmostOverquotaColor(const QColor &color)
{
    almostOverquotaColor = color;
    update();
}

QColor CircularUsageProgressBar::getOverquotaColor() const
{
    return overquotaColor;
}

void CircularUsageProgressBar::setOverquotaColor(const QColor &color)
{
    overquotaColor = color;
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
        totalValueUnkown = false;
        textValue = tr("[A]%").replace(QStringLiteral("[A]"), QString::number(value));
        progressBarValue = value;

        if (value >= CircularUsageProgressBar::MAXVALUE)
        {
            currentColor = overquotaColor;
            setPenColor(foregroundPen, overquotaColor, false);
        }
        else if (value >= ALMOSTOVERQUOTA_VALUE)
        {
            currentColor = almostOverquotaColor;
            setPenColor(foregroundPen, almostOverquotaColor, false);
        }
        else
        {
            currentColor = foregroundColor;
            setPenColor(foregroundPen, currentColor, false);
        }
        update();
    }
}

void CircularUsageProgressBar::setBarTotalValueUnkown(int value, const QColor &color)
{
    totalValueUnkown = true;
    progressBarValue = value;
    currentColor = color;
    setPenColor(foregroundPen, currentColor, false);
    update();
}

void CircularUsageProgressBar::setEmptyBarTotalValueUnknown()
{
    setBarTotalValueUnkown(0, foregroundColor);
}

void CircularUsageProgressBar::setFullBarTotalValueUnkown()
{
    setBarTotalValueUnkown(CircularUsageProgressBar::MAXVALUE, overquotaColor);
}

QColor CircularUsageProgressBar::getForegroundColor() const
{
    return foregroundColor;
}

void CircularUsageProgressBar::setForegroundColor(const QColor &color)
{
    foregroundColor = color;
    update();
}

QColor CircularUsageProgressBar::getBackgroundColor() const
{
    return backgroundColor;
}

void CircularUsageProgressBar::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;
    update();
}
