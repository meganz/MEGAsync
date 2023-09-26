#include "WordWrapLabel.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QResizeEvent>

#include <Utilities.h>

const int MINIMUM_DOC_HEIGHT = 3;
const int MINMUM_HEIGHT = 16;

//This event is propagated from child to parent, this is why it is used
const QEvent::Type WordWrapLabel::HeightAdapted = QEvent::WhatsThisClicked;

WordWrapLabel::WordWrapLabel(QWidget* parent)
    : QTextBrowser(parent),
      mLinkActivated(false)
{
    setFrameStyle(QFrame::NoFrame);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    setCursor(parent->cursor());
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setOpenLinks(false);

    parent->installEventFilter(this);
    viewport()->installEventFilter(this);

    document()->setDocumentMargin(0);
    setFixedHeight(MINMUM_HEIGHT);

    connect(this, &WordWrapLabel::anchorClicked, this, &WordWrapLabel::onLinkActivated);
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
                qApp->postEvent(this, new QEvent(HeightAdapted));
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

bool WordWrapLabel::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::CursorChange)
    {
        if(viewport()->cursor().shape() != parentWidget()->cursor().shape())
        {
            setCursor(parentWidget()->cursor());
        }
    }
    return QTextEdit::eventFilter(obj, event);
}

void WordWrapLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QTextBrowser::mousePressEvent(ev);

    //In order to propagate the mouse release event
    //Otherwise, the QTextBrowser hijacks it even if you don´t press of the link
    if(!mLinkActivated)
    {
        QWidget::mousePressEvent(ev);
    }

    //Reset for next click
    mLinkActivated = false;
}

void WordWrapLabel::onLinkActivated(const QUrl &link)
{
    mLinkActivated = true;
    Utilities::openUrl(link);
}

void WordWrapLabel::setCursor(const QCursor& cursor)
{
    QTextEdit::setCursor(cursor);
    viewport()->setCursor(cursor);
}
