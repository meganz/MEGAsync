#include "CircularUsageProgressBar.h"

const int CircularUsageProgressBar::MINVALUE;
const int CircularUsageProgressBar::MAXVALUE;

static const QColor DEFAULT_OK_LIGHT       ("#00BEA4");
static const QColor DEFAULT_OK_DARK        ("#009985");
static const QColor DEFAULT_BAR_BACKGROUND ("#E5E5E5");
static const QColor DEFAULT_FULL_LIGHT     ("#EE7272");
static const QColor DEFAULT_FULL_DARK      ("#DB080F");
static const QColor DEFAULT_WARN_LIGHT     ("#FFAF00");
static const QColor DEFAULT_WARN_DARK      ("#F06F01");
static const QColor DEFAULT_TEXT_COLOR     ("#000000");

constexpr qreal LIGHT_ANGLE    (1.);
constexpr qreal DARK_ANGLE     (0.);
constexpr qreal GRADIENT_ANGLE (90.);

CircularUsageProgressBar::CircularUsageProgressBar(QWidget *parent) :
    QWidget       (parent),
    mPbValue      (0),
    mPenWidth     (0.),
    mOuterRadius  (0.),
    mState        (STATE_OK),
    mPbBgColor    (DEFAULT_BAR_BACKGROUND),
    mPbGradient   (&mOkPbGradient),
    mMarkWarning  (QStringLiteral(":/images/icon_warning_24.png")),
    mMarkFull     (QStringLiteral(":/images/icon_error_24.png")),
    mDynTrsfOk    (QStringLiteral(":/images/dynamic_transfer_icon_32.png")),
    mDynTrsfFull  (QStringLiteral(":/images/dynamic_transfer_overquota_icon.png")),
    mNoTotalValue (true)
{
    // Init Gradients
    mOkPbGradient.setAngle(GRADIENT_ANGLE);
    mOkPbGradient.setColorAt(DARK_ANGLE, DEFAULT_OK_DARK);
    mOkPbGradient.setColorAt(LIGHT_ANGLE, DEFAULT_OK_LIGHT);

    mWarnPbGradient.setAngle(GRADIENT_ANGLE);
    mWarnPbGradient.setColorAt(DARK_ANGLE, DEFAULT_WARN_DARK);
    mWarnPbGradient.setColorAt(LIGHT_ANGLE, DEFAULT_WARN_LIGHT);

    mFullPbGradient.setAngle(GRADIENT_ANGLE);
    mFullPbGradient.setColorAt(DARK_ANGLE, DEFAULT_FULL_DARK);
    mFullPbGradient.setColorAt(LIGHT_ANGLE, DEFAULT_FULL_LIGHT);

    setPenColor(mBgPen, mPbBgColor);
    setPenGradient(mFgPen, *mPbGradient);
}

void CircularUsageProgressBar::paintEvent(QPaintEvent*)
{
    constexpr int padingPixels (4);
    const double updatedOuterRadius (std::min(width(), height()) - padingPixels);

    if (updatedOuterRadius != mOuterRadius)
    {
        mOuterRadius = updatedOuterRadius;
        mPenWidth    = mOuterRadius / 352. * 37.;

        // Update baseRect dimensions
        mBaseRect.setX(mPenWidth / 2.);
        mBaseRect.setY((mPenWidth / 2.) + (padingPixels / 2.));
        mBaseRect.setWidth(mOuterRadius - mPenWidth);
        mBaseRect.setHeight(mBaseRect.width());

        setPenColor(mBgPen, mPbBgColor, false);
        mBgPen.setWidth(static_cast<int>(mPenWidth));

        setPenGradient(mFgPen, *mPbGradient, false);
        mFgPen.setWidth(static_cast<int>(mPenWidth));
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::TextAntialiasing
                           | QPainter::SmoothPixmapTransform);

    painter.fillRect(mBaseRect, Qt::NoBrush);

    // Draw white disc background
    painter.setBrush(Qt::white);
    painter.drawEllipse(mBaseRect);

    // Draw background progress bar
    painter.setPen(mBgPen);
    painter.drawArc(mBaseRect, 90 * 16, -(CircularUsageProgressBar::MAXVALUE * 360 * 16) / 100);

    // Draw value arc
    painter.setPen(mFgPen);
    painter.drawArc(mBaseRect, 90 * 16, -(mPbValue * 360 * 16) / 100);

    //Draw percentage text or dynamic transfer icon
    const double innerRadius (mOuterRadius - mPenWidth / 2.);
    const double delta       ((mOuterRadius - innerRadius) / 2.);
    const QRectF innerRect   (delta, delta + padingPixels / 2., innerRadius, innerRadius);

    if (mNoTotalValue)
    {
        constexpr QSize dynamicIconNativeSizePixels (32, 32);

        QRectF dynamicIconRect(QPoint(0, 0), dynamicIconNativeSizePixels);
        dynamicIconRect.moveCenter(innerRect.center());

        const QIcon& dynamicQuotaIcon (mState == STATE_OVER ? mDynTrsfFull : mDynTrsfOk);

        painter.drawPixmap(dynamicIconRect.toRect(),
                           dynamicQuotaIcon.pixmap(dynamicIconNativeSizePixels));
    }
    else
    {
        drawText(painter, innerRect, innerRadius, mPbValue);
    }

    // If value higher than warning threshold show warning image
    if (mState != STATE_OK)
    {
        constexpr double nativeOuterRadius (48.);
        constexpr int    iconSizePixels    (24);
        constexpr int    iconPaddingX      (3);
        constexpr int    iconPaddingY      (4);

        const double pixmapTotalSideLength ((mOuterRadius / nativeOuterRadius) * iconSizePixels);

        const int x (static_cast<int>(mOuterRadius - (pixmapTotalSideLength / 2.) - iconPaddingX));
        const int y (padingPixels / 2 - iconPaddingY);

        const int sideLength(static_cast<int>(pixmapTotalSideLength));

        const QIcon&  icon   (mState == STATE_OVER ? mMarkFull : mMarkWarning);
        const QPixmap pixmap (icon.pixmap(iconSizePixels, iconSizePixels));

        painter.drawPixmap(x, y, sideLength, sideLength, pixmap);
    }
}

void CircularUsageProgressBar::drawText(QPainter& p, const QRectF& innerRect, double innerRadius,
                                        double value)
{
    QFont f(font());
    const double factor_decrease (0.86);
    double factor (1.);

    double aux (value);

    while (aux >= 1000.)
    {
        factor *= factor_decrease;
        aux    /= 10.;
    }

    double pixelSize (innerRadius * 0.3);

    f.setPixelSize(std::max(5, static_cast<int>(pixelSize * factor)));
    f.setFamily(QString::fromUtf8("Lato"));

    p.setFont(f);

    const auto penColor(mState == STATE_OK ? DEFAULT_TEXT_COLOR
                                           : mPbGradient->stops().begin()->second);
    p.setPen(penColor);

    p.drawText(innerRect, Qt::AlignCenter, mTextValue);
}

void CircularUsageProgressBar::setPenColor(QPen& pen, QColor color, bool forceRepaint)
{
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);

    if (forceRepaint)
    {
        update();
    }
}

void CircularUsageProgressBar::setPenGradient(QPen& pen, QConicalGradient& gradient,
                                              bool forceRepaint)
{
    pen.setCapStyle(Qt::FlatCap);
    gradient.setCenter(mBaseRect.center());
    pen.setBrush(gradient);

    if (forceRepaint)
    {
        update();
    }
}

void CircularUsageProgressBar::setValue(int value)
{
    if ( value != mPbValue || mNoTotalValue)
    {
        mPbValue = std::max(CircularUsageProgressBar::MINVALUE, value);

        mNoTotalValue = false;
        mTextValue = tr("[A]%").replace(QStringLiteral("[A]"), QString::number(mPbValue));
    }

    switch (mState)
    {
        case STATE_OK:
        {
            mPbGradient = &mOkPbGradient;
            break;
        }
        case STATE_WARNING:
        {
            mPbGradient = &mWarnPbGradient;
            break;
        }
        case STATE_OVER:
        {
            mPbGradient = &mFullPbGradient;
            break;
        }
    }

    setPenGradient(mFgPen, *mPbGradient);
}

void CircularUsageProgressBar::setBarTotalValueUnkown(int value, QConicalGradient* gradient)
{
    mNoTotalValue = true;
    mPbValue = value;
    mPbGradient = gradient;
    setPenGradient(mFgPen, *mPbGradient);
}

void CircularUsageProgressBar::setState(STATE state)
{
    mState = state;
    // Refresh color if necessary
    setValue(mPbValue);
}

void CircularUsageProgressBar::setTotalValueUnknown(bool isEmptyBar)
{
    if (isEmptyBar)
    {
        setBarTotalValueUnkown(0, &mOkPbGradient);
    }
    else
    {
        setBarTotalValueUnkown(CircularUsageProgressBar::MAXVALUE, &mFullPbGradient);
    }
}

void CircularUsageProgressBar::setProgressBarGradient(QColor light, QColor dark)
{
    mOkPbGradient.stops().clear();
    mOkPbGradient.setColorAt(DARK_ANGLE, dark);
    mOkPbGradient.setColorAt(LIGHT_ANGLE, light);
}
