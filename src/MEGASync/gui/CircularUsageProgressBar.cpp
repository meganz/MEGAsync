#include "CircularUsageProgressBar.h"

static const QColor DEFAULT_OK_LIGHT       ("#00BEA4");
static const QColor DEFAULT_OK_DARK("#009985");
static const QColor DEFAULT_WARN_LIGHT     ("#FFAF00");
static const QColor DEFAULT_WARN_DARK      ("#F06F01");
static const QColor DEFAULT_FULL_LIGHT("#EE7272");
static const QColor DEFAULT_FULL_DARK("#DB080F");

static const QColor DEFAULT_BAR_BACKGROUND("#E5E5E5");
static const QColor DEFAULT_TEXT_COLOR     ("#000000");

static constexpr int MAXVALUE = 100;
static constexpr int MINVALUE = 0;

static constexpr qreal LIGHT_ANGLE(1.);
static constexpr qreal DARK_ANGLE(0.);
static constexpr qreal GRADIENT_ANGLE(90.);

CircularUsageProgressBar::CircularUsageProgressBar(QWidget* parent):
    QWidget(parent),
    mPbValue(0),
    mPenWidth(0.),
    mOuterRadius(0.),
    mState(STATE_OK),
    mPbBgColor(DEFAULT_BAR_BACKGROUND),
    mBgColor(Qt::transparent),
    mOkStateTextColor(DEFAULT_TEXT_COLOR),
    mPbGradient(&mOkPbGradient),
    mMarkWarning(QStringLiteral(":/images/icon_warning_24.png")),
    mMarkFull(QStringLiteral(":/images/icon_error_24.png")),
    mDynTrsfOk(QStringLiteral(":/images/dynamic_transfer_icon_32.png")),
    mDynTrsfFull(QStringLiteral(":/images/dynamic_transfer_overquota_icon.png")),
    mNoTotalValue(true)
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

    mOuterRadius = updatedOuterRadius;
    mPenWidth = mOuterRadius / 352. * 22.;

    // Update baseRect dimensions
    mBaseRect.setX(mPenWidth / 2.);
    mBaseRect.setY((mPenWidth / 2.) + (padingPixels / 2.));
    mBaseRect.setWidth(mOuterRadius - mPenWidth);
    mBaseRect.setHeight(mBaseRect.width());

    setPenColor(mBgPen, mPbBgColor, false);
    mBgPen.setWidth(static_cast<int>(mPenWidth));

    setPenGradient(mFgPen, *mPbGradient, false);
    mFgPen.setWidth(static_cast<int>(mPenWidth));

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::TextAntialiasing
                           | QPainter::SmoothPixmapTransform);

    painter.fillRect(mBaseRect, Qt::NoBrush);

    // Draw disc background
    painter.setBrush(mBgColor);
    painter.drawEllipse(mBaseRect);

    // Draw background progress bar
    painter.setPen(mBgPen);
    painter.drawArc(mBaseRect, 90 * 16, -(MAXVALUE * 360 * 16) / 100);

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

    const auto penColor(mState == STATE_OK ? mOkStateTextColor :
                                             mPbGradient->stops().begin()->second);
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
        mPbValue = std::max(MINVALUE, value);

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
        setBarTotalValueUnkown(MAXVALUE, &mFullPbGradient);
    }
}

void CircularUsageProgressBar::setLightOkProgressBarColor(const QString& color)
{
    setProgressBarColors(color, STATE_OK, true);
}

void CircularUsageProgressBar::setDarkOkProgressBarColor(const QString& color)
{
    setProgressBarColors(color, STATE_OK, false);
}

void CircularUsageProgressBar::setLightWarnProgressBarColor(const QString& color)
{
    setProgressBarColors(color, STATE_WARNING, true);
}

void CircularUsageProgressBar::setDarkWarnProgressBarColor(const QString& color)
{
    setProgressBarColors(color, STATE_WARNING, false);
}

void CircularUsageProgressBar::setLightFullProgressBarColor(const QString& color)
{
    setProgressBarColors(color, STATE_OVER, true);
}

void CircularUsageProgressBar::setDarkFullProgressBarColor(const QString& color)
{
    setProgressBarColors(color, STATE_OVER, false);
}

void CircularUsageProgressBar::setProgressBarColors(const QString& color, STATE state, bool light)
{
    switch (state)
    {
        case STATE_OK:
        {
            mOkPbGradient.setColorAt(light ? LIGHT_ANGLE : DARK_ANGLE, color);
            break;
        }
        case STATE_WARNING:
        {
            mWarnPbGradient.setColorAt(light ? LIGHT_ANGLE : DARK_ANGLE, color);
            break;
        }
        case STATE_OVER:
        {
            mFullPbGradient.setColorAt(light ? LIGHT_ANGLE : DARK_ANGLE, color);
            break;
        }
    }

    emit colorChanged();
}

void CircularUsageProgressBar::setOuterCircleBackgroundColor(const QString& color)
{
    if (mPbBgColor != color)
    {
        mPbBgColor = color;

        emit colorChanged();
    }
}

void CircularUsageProgressBar::setInnerCircleBackgroundColor(const QString& color)
{
    if (mBgColor != color)
    {
        mBgColor = color;

        emit colorChanged();
    }
}

void CircularUsageProgressBar::setOkStateTextColor(const QString& color)
{
    if (mOkStateTextColor != color)
    {
        mOkStateTextColor = color;

        emit colorChanged();
    }
}
