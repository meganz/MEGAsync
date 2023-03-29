#include "QmlDialogWrapper.h"
#include "MegaApplication.h"
#include <QQmlProperty>
#include <QWindow>


QMLComponent::QMLComponent(QObject *parent)
    : QObject(parent)
{
}

QMLComponent::~QMLComponent()
{

}

QQmlEngine *QMLComponent::getEngine()
{
    return MegaSyncApp->qmlEngine();
}

QmlDialogWrapperBase::QmlDialogWrapperBase(QObject *parent)
    : QObject(parent)
    , mWindow(nullptr)
    , mResult(QDialog::Rejected)
{

}

QmlDialogWrapperBase::~QmlDialogWrapperBase()
{
    mWindow->deleteLater();
}

Qt::WindowModality QmlDialogWrapperBase::windowModality()
{
    return qvariant_cast<Qt::WindowModality>(QQmlProperty::read(mWindow, QString::fromUtf8("modality")));
}

void QmlDialogWrapperBase::setWindowModality(Qt::WindowModality modality)
{
    QQmlProperty::write(mWindow, QString::fromUtf8("modality"), modality);
}

Qt::WindowFlags QmlDialogWrapperBase::windowFlags()
{
    return mWindow->flags();
}

void QmlDialogWrapperBase::setWindowFlags(Qt::WindowFlags flags)
{
    mWindow->setFlags(flags);
}

void QmlDialogWrapperBase::setWindowState(Qt::WindowState state)
{
    switch(state)
    {
    case Qt::WindowState::WindowMaximized:
        mWindow->showMaximized();
        break;
    case Qt::WindowState::WindowFullScreen:
        mWindow->showFullScreen();
        break;
    case Qt::WindowState::WindowMinimized:
        mWindow->showMinimized();
        break;
    case Qt::WindowState::WindowNoState:
        mWindow->showNormal();
        break;
    case Qt::WindowState::WindowActive:
        mWindow->requestActivate();
    }
}

void QmlDialogWrapperBase::move(const QPoint &point)
{
    QRect rect = geometry();
    rect.moveTopLeft(point);
    mWindow->setGeometry(rect);
}

void QmlDialogWrapperBase::showMaximized()
{
    mWindow->showMaximized();
}

void QmlDialogWrapperBase::setGeometry(const QRect &geometry)
{
    mWindow->setGeometry(geometry);
}

QRect QmlDialogWrapperBase::geometry()
{
    return mWindow->geometry();
}

bool QmlDialogWrapperBase::isMaximized()
{
    QWindow::Visibility state = qvariant_cast<QWindow::Visibility>(QQmlProperty::read(mWindow, QString::fromUtf8("visibility")));
    return (state & QWindow::Maximized);
}

void QmlDialogWrapperBase::show()
{
    setWindowState(mWindow->windowState());
}

void QmlDialogWrapperBase::activateWindow()
{
    mWindow->requestActivate();
}

void QmlDialogWrapperBase::raise()
{
    mWindow->raise();
}

int QmlDialogWrapperBase::result()
{
    return mResult;
}

void QmlDialogWrapperBase::close()
{
    mWindow->close();
}

void QmlDialogWrapperBase::accept()
{
    mResult = QDialog::Accepted;
    close();
}

void QmlDialogWrapperBase::reject()
{
    mResult = QDialog::Rejected;
    close();
}

void QmlDialogWrapperBase::onWindowFinished()
{
    if (mResult == QDialog::Accepted)
    {
        emit accepted();
    }
    else if (mResult == QDialog::Rejected)
    {
        emit rejected();
    }
    emit finished(mResult);
}
