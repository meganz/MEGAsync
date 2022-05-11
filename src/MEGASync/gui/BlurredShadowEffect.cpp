#include "BlurredShadowEffect.h"

#include <QGraphicsDropShadowEffect>

QGraphicsEffect* CreateBlurredShadowEffect()
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(20);
    effect->setXOffset(0);
    effect->setYOffset(2);
    effect->setColor(QColor(215, 214, 213));
    return effect;
}
