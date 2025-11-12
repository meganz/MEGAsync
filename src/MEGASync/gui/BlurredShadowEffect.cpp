#include "BlurredShadowEffect.h"

#include <QGraphicsDropShadowEffect>

QGraphicsEffect* CreateBlurredShadowEffect(qreal radius)
{
    return CreateBlurredShadowEffect(QColor(0, 0, 0, 38), radius);
}

QGraphicsEffect*
    CreateBlurredShadowEffect(const QColor& shadowColor, qreal radius, int XOffset, int YOffset)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(radius);
    effect->setXOffset(XOffset);
    effect->setYOffset(YOffset);
    effect->setColor(shadowColor);
    return effect;
}
