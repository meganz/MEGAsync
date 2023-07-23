#include "WordWrapLabel.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>

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

    document()->setDocumentMargin(0);
}

void WordWrapLabel::setText(const QString &text)
{
    QTextEdit::setText(text);
    adaptHeight();
}

QSize WordWrapLabel::sizeHint() const
{
    QSize size = document()->size().toSize();
    size.setHeight(size.height() + 3);

    return size;
}

void WordWrapLabel::adaptHeight()
{
    QSize docSize = document()->size().toSize();
    if(docSize.isValid())
    {
        if((docSize.height() + 3) != (height()))
        {
            setFixedHeight(docSize.height() + 3);
            qApp->postEvent(this, new QEvent(QEvent::WhatsThisClicked));
        }
    }
}

void WordWrapLabel::resizeEvent (QResizeEvent *event)
{
    /*
      * If the widget has been resized then the size hint will
      * also have changed.  Call updateGeometry to make sure
      * any layouts are notified of the change.
      */
    updateGeometry();

    adaptHeight();

    QTextEdit::resizeEvent(event);
}
