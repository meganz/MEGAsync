#include "HighDpiResize.h"
#include <QEvent>
#include <QTimer>
#include <assert.h>

void HighDpiResize::init(QDialog* d)
{
#ifdef WIN32
    dialog = d;
    d->installEventFilter(this);
#endif
}

bool HighDpiResize::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ScreenChangeInternal)
    {
        queueRedraw();
    }
    return QObject::eventFilter(obj, event);
}

void HighDpiResize::queueRedraw()
{
#ifdef WIN32
    // waiting one second means we don't cause the window to be resized multiple times when dragged from one screen to another with a different scaling
    QTimer::singleShot(1000, this, SLOT(forceRedraw()));
#endif
}

void HighDpiResize::forceRedraw()
{
    // When dragging this window from one screen to another with a different scaling ratio 
    // (at least in windows 10 with qt 5.6.3), occastionally the window does not resize properly, 
    // leaving 100% controls inside a 200% window or vice versa.
    // This resize() command triggers reevaluation of the window size, which is fixed size anyway.
    if (dialog)
    {
        if (dialog->minimumWidth() == dialog->maximumWidth() && dialog->minimumHeight() == dialog->maximumHeight())
        {
            dialog->resize(1, 1);  // adjustSize() didn't work for the infoDialog when it appears at 100% on a 200% screen
            QRect r = dialog->rect();
            r.setRight(r.right() + 100);
            dialog->update(r); // going from 200% to 100% leaves some artifacts otherwise, some of the background is not repainted, very visible in About dialog
        }
    }
    else
    {
        // adjustsize() for sizable windows does something odd, eg upload dialog goes from landscape shape to narrow portrait
        // so try changing the size down one pixel and then up again
        QSize s = dialog->size();
        dialog->resize(s.width() - 1, s.height() - 1);
        dialog->resize(s);
    }
}
