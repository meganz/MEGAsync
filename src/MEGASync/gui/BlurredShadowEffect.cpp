#include "BlurredShadowEffect.h"

#include <QGraphicsDropShadowEffect>

QGraphicsEffect* CreateBlurredShadowEffect(qreal radius)
{
    return CreateBlurredShadowEffect(QColor(0, 0, 0, 38), radius);
}

QGraphicsEffect* CreateBlurredShadowEffect(const QColor& shadowColor, qreal radius)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(radius);
    effect->setXOffset(0);
    effect->setYOffset(1);
    effect->setColor(shadowColor);
    return effect;
}
