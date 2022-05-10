#include "BlurredShadowEffect.h"

#include <QGraphicsDropShadowEffect>

QGraphicsEffect* CreateBlurredShadowEffect()
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(20);
    effect->setXOffset(2);
    effect->setYOffset(1);
    effect->setColor(QColor(215, 214, 213));
    return effect;
}
