#ifndef BLURREDSHADOWEFFECT_H
#define BLURREDSHADOWEFFECT_H

#include <QGraphicsEffect>

QGraphicsEffect* CreateBlurredShadowEffect(qreal radius = 2.0);
QGraphicsEffect* CreateBlurredShadowEffect(const QColor& shadowColor, qreal radius = 2.0);

#endif // BLURREDSHADOWEFFECT_H
