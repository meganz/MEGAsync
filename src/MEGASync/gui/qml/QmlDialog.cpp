#include "QmlDialog.h"
#include <QEvent>
#include <QScreen>
#include <QApplication>

QmlDialog::QmlDialog(QWindow *parent)
    : QQuickWindow(parent)
{
}

QmlDialog::~QmlDialog()
{
}

bool QmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        emit finished();
    }
    return QQuickWindow::event(evnt);
}
