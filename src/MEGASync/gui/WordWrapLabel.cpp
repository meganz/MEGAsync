#include "WordWrapLabel.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QResizeEvent>

const int MINIMUM_DOC_HEIGHT = 3;
const int MINMUM_HEIGHT = 16;

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
    setFixedHeight(MINMUM_HEIGHT);
}

void WordWrapLabel::setText(const QString &text)
{
    QTextEdit::setText(text);
    adaptHeight();
}

void WordWrapLabel::adaptHeight(bool sendEvent)
{
    setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
    QSize docSize = document()->size().toSize();
    if(docSize.isValid() && docSize.height() > MINIMUM_DOC_HEIGHT)
    {
        if((docSize.height() + 3) != (height()))
        {
            setFixedHeight(docSize.height() + 3);
            if(sendEvent)
            {
                qApp->postEvent(this, new QEvent(QEvent::WhatsThisClicked));
            }
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

    if(!toPlainText().isEmpty())
    {
        adaptHeight(true);
    }

    QTextEdit::resizeEvent(event);
}
