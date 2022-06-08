#include "BlurredShadowEffect.h"

#include <QGraphicsDropShadowEffect>

QGraphicsEffect* CreateBlurredShadowEffect()
{
    return CreateBlurredShadowEffect(QColor(0, 0, 0, 38));
}

QGraphicsEffect* CreateBlurredShadowEffect(const QColor& shadowColor)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(2);
    effect->setXOffset(0);
    effect->setYOffset(1);
    effect->setColor(shadowColor);
    return effect;
}
