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
    QTimer::singleShot(100, this, SLOT(forceRedraw()));
#endif
}

void HighDpiResize::forceRedraw()
{
    // When dragging this window from one screen to another with a different scaling ratio 
    // (at least in windows 10 with qt 5.6.3), occastionally the window does not resize properly, 
    // leaving 100% controls inside a 200% window or vice versa.
    // This resize() command triggers reevaluation of the window size, which is fixed size anyway.
    //assert(dialog->minimumWidth() == dialog->maximumWidth());
    //assert(dialog->minimumHeight() == dialog->maximumHeight());
    if (dialog) dialog->adjustSize();
}
