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

    if (singleline)
    {
        QRect r = contentsRect();
        r.setY( static_cast<int>(height() / 2.0 - font().pixelSize() / 2.0) );
        QString elidedText = fontMetrics.elidedText(text(), Qt::ElideRight, r.width());
        didElide = (elidedText != text());
        painter.drawText(r, elidedText);
        QFrame::paintEvent(event);
    }
    else
    {
        int lineSpacing = fontMetrics.lineSpacing();
        int y = 0;

        QTextLayout textLayout(content, painter.font());
        textLayout.beginLayout();
        for(;;)
        {
            QTextLine line = textLayout.createLine();
            if (!line.isValid())
            {
                break;
            }

            line.setLineWidth(width());
            int nextLineY = y + lineSpacing;

            if (height() >= nextLineY + lineSpacing)
            {
                line.draw(&painter, QPoint(0, y));
                y = nextLineY;
            }
            else
            {
                QString lastLine = content.mid(line.textStart());
                QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width());
                painter.drawText(QPoint(0, y + fontMetrics.ascent()), elidedLastLine);
                line = textLayout.createLine();
                didElide = line.isValid();
                break;
            }
        }
        textLayout.endLayout();
    }
    if (didElide != elided)
    {
        elided = didElide;
        emit elisionChanged(didElide);
    }
}

bool ElidedLabel::getSingleline() const
{
    return singleline;
}

void ElidedLabel::setSingleline(bool value)
{
    singleline = value;
}
