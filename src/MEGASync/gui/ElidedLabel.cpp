#include "ElidedLabel.h"

#include <QPainter>
#include <QTextLayout>
#include <QDebug>

ElidedLabel::ElidedLabel(QWidget *parent)
    : QFrame(parent)
    , elided(false)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ElidedLabel::setText(const QString &newText)
{
    content = newText;
    update();
}

void ElidedLabel::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    QFontMetrics fontMetrics = painter.fontMetrics();

    bool didElide = false;

    QRect r = contentsRect();
    r.setY( height() / 2.0 - font().pixelSize() / 2.0 );
    QString elidedText = fontMetrics.elidedText(text(), Qt::ElideRight, r.width());
    didElide = (elidedText != text());
    painter.drawText(r, elidedText);

    if (didElide != elided)
    {
        elided = didElide;
        emit elisionChanged(didElide);
    }
}
