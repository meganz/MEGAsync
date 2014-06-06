#ifndef MEGAPROXYSTYLE_H
#define MEGAPROXYSTYLE_H

#include <QProxyStyle>
#include <QPainter>

class MegaProxyStyle : public QProxyStyle
{
    Q_OBJECT

public:
    virtual void drawComplexControl ( ComplexControl control, const QStyleOptionComplex * option, QPainter * painter, const QWidget * widget = 0 ) const;
    virtual void drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;
    virtual void drawItemPixmap ( QPainter * painter, const QRect & rect, int alignment, const QPixmap & pixmap ) const;
    virtual void drawItemText ( QPainter * painter, const QRect & rect, int flags, const QPalette & pal, bool enabled, const QString & text, QPalette::ColorRole textRole = QPalette::NoRole ) const;
    virtual void drawPrimitive ( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;
};

#endif // MEGAPROXYSTYLE_H
