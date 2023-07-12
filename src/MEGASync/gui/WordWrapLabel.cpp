#include "WordWrapLabel.h"

#include <QDebug>

WordWrapLabel::WordWrapLabel(QWidget* parent)
    : QTextEdit(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setTextInteractionFlags(Qt::NoTextInteraction);
    setCursor(Qt::ArrowCursor);
    viewport()->setCursor(Qt::ArrowCursor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void WordWrapLabel::setText(const QString &text)
{
    QTextEdit::setText(text);

    QSize docSize = document()->size().toSize();
    if(docSize.isValid())
    {
        if(docSize.height() != (height() + 3))
        {
            docSize.setHeight(docSize.height() + 3);
            setFixedHeight(docSize.height());
        }
    }
}

QSize WordWrapLabel::sizeHint() const
{
    QSize size = document()->size().toSize();
    size.setHeight(size.height() + 3);

    return size;
}

void WordWrapLabel::resizeEvent (QResizeEvent *event)
{
    /*
      * If the widget has been resized then the size hint will
      * also have changed.  Call updateGeometry to make sure
      * any layouts are notified of the change.
      */
    updateGeometry();


    QSize docSize = document()->size().toSize();
    if(docSize.isValid())
    {
        if(docSize.height() != (height() + 3))
        {
            docSize.setHeight(docSize.height() + 3);
            setFixedHeight(docSize.height());
        }
    }

    QTextEdit::resizeEvent(event);
}
