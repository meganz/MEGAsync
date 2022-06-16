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

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;

    QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option,
                       const QWidget *widget) const override;

    using QProxyStyle::polish;
    void polish(QWidget *widget) override;
    void polish(QPalette &pal) override;
    void polish(QApplication *app) override;

    void unpolish(QWidget *widget) override;
    void unpolish(QApplication *app) override;

protected:
    bool event(QEvent *e) override;
};

#endif // MEGAPROXYSTYLE_H
