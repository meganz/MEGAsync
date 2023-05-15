#ifndef HIGHDPIRESIZE_H
#define HIGHDPIRESIZE_H

#include <QDialog>
#include <QPointer>
#include <QEvent>
#include <QTimer>
#include <QDebug>

class HighDpiResizeBase : public QObject
{
    Q_OBJECT
public:
    HighDpiResizeBase() = default;
    HighDpiResizeBase(QObject* d) : QObject(d)
    {}


public slots:
    virtual void forceRedraw() = 0;

};

template <class Dialog>
class HighDpiResize : public HighDpiResizeBase
{
    //Q_OBJECT

public:
    HighDpiResize() = default;

    HighDpiResize(Dialog* d) : HighDpiResizeBase(d)
    {
          init(d);
    }

    void init(Dialog* d)
    {
    #if defined(WIN32) || defined(Q_OS_LINUX)
        dialog = d;
        d->installEventFilter(this);
    #endif
    }

    void queueRedraw()
    {
    #if defined(WIN32)
        // waiting one second means we don't cause the window to be resized multiple times when dragged from one screen to another with a different scaling
        QTimer::singleShot(1000, this, SLOT(forceRedraw()));
    #elif defined(Q_OS_LINUX)
        // in linux multiple resizes seems more or less bearable, whereas the 1s delay appears buggy when moving between screens
        QTimer::singleShot(100, this, SLOT(forceRedraw()));
    #endif
    }

    void forceRedraw()
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
                else
                {
                    // adjustsize() for sizable windows does something odd, eg upload dialog goes from landscape shape to narrow portrait
                    // so try changing the size down one pixel and then up again
                    QSize s = dialog->size();
                    dialog->resize(s.width() - 1, s.height() - 1);
                    dialog->resize(s);
                }
            }
    }


protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
    #if QT_VERSION >= 0x050000
        qDebug()<<event->type();
        if (event->type() == QEvent::ScreenChangeInternal)
        {
            queueRedraw();
        }
    #endif
        return QObject::eventFilter(obj, event);
    }
private:
    QPointer<Dialog> dialog = nullptr;
};

#endif // HIGHDPIRESIZE_H
