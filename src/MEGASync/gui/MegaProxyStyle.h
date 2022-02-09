#ifndef MEGAPROXYSTYLE_H
#define MEGAPROXYSTYLE_H

#include <QProxyStyle>
#include <QPainter>

class MegaProxyStyle : public QProxyStyle
{
    Q_OBJECT

public:
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex * option,
                              QPainter * painter, const QWidget * widget = 0) const override;

    void drawControl(ControlElement element, const QStyleOption * option,
                       QPainter * painter, const QWidget * widget = 0) const override;

    void drawItemPixmap(QPainter * painter, const QRect & rect, int alignment,
                          const QPixmap & pixmap) const override;

    void drawItemText (QPainter * painter, const QRect & rect, int flags,
                        const QPalette & pal, bool enabled, const QString & text,
                       QPalette::ColorRole textRole = QPalette::NoRole) const override;

    void drawPrimitive (PrimitiveElement element, const QStyleOption * option,
                         QPainter * painter, const QWidget * widget = 0) const override;

    int pixelMetric(PixelMetric metric, const QStyleOption * option,
                    const QWidget * widget = 0) const override;

    QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option,
                       const QWidget *widget) const override;

    using QProxyStyle::polish;
    void polish(QWidget *widget) override;
};

#endif // MEGAPROXYSTYLE_H
