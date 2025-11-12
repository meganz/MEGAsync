#include "QmlDialogWrapper.h"

#include <QQmlProperty>
#include <QWindow>

namespace
{
const QLatin1String NAMESPACE_SEPARATOR("::");
}

// ************************************************************************************************
// * QMLComponent
// ************************************************************************************************

QList<QObject*> QMLComponent::getInstancesFromContext()
{
    return QList<QObject*>();
}

QString QMLComponent::contextName() const
{
    // Get Child class name.
    QString className(QString::fromLatin1(metaObject()->className()));

    // Remove namespace prefix if necessary.
    int index(className.lastIndexOf(NAMESPACE_SEPARATOR));
    if (index != -1)
    {
        // Ignore namespace separator.
        className = className.mid(index + NAMESPACE_SEPARATOR.size());
    }

    return className;
}

// ************************************************************************************************
// * QmlDialogWrapperBase
// ************************************************************************************************

QmlDialogWrapperBase::QmlDialogWrapperBase(QWidget* parent):
    QWidget(parent),
    mWindow(nullptr),
    mResult(QDialog::Rejected)
{}

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
        {
            mWindow->showMaximized();
            break;
        }
        case Qt::WindowState::WindowFullScreen:
        {
            mWindow->showFullScreen();
            break;
        }
        case Qt::WindowState::WindowMinimized:
        {
            mWindow->showMinimized();
            break;
        }
        case Qt::WindowState::WindowNoState:
        {
            mWindow->showNormal();
            break;
        }
        case Qt::WindowState::WindowActive:
        {
            mWindow->requestActivate();
            break;
        }
        default:
        {
            return;
        }
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

void QmlDialogWrapperBase::showNormal()
{
    mWindow->showNormal();
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
    return (mWindow->windowState() & Qt::WindowState::WindowMaximized);
}

bool QmlDialogWrapperBase::isMinimized()
{
    return (mWindow->windowState() & Qt::WindowState::WindowMinimized);
}

bool QmlDialogWrapperBase::isVisible()
{
    return mWindow->isVisible();
}

void QmlDialogWrapperBase::hide()
{
    return mWindow->hide();
}

void QmlDialogWrapperBase::show()
{
   setWindowState(mWindow->windowState());
}

void QmlDialogWrapperBase::activateWindow()
{
   mWindow->requestActivate();
}

QWindow* QmlDialogWrapperBase::windowHandle()
{
    return mWindow;
}

int QmlDialogWrapperBase::minimumWidth()
{
    return mWindow->minimumWidth();
}

int QmlDialogWrapperBase::maximumWidth()
{
    return mWindow->maximumWidth();
}

int QmlDialogWrapperBase::maximumHeight()
{
    return mWindow->maximumHeight();
}

int QmlDialogWrapperBase::minimumHeight()
{
    return mWindow->minimumHeight();
}

QRect QmlDialogWrapperBase::rect()
{
    return mWindow->geometry();
}

void QmlDialogWrapperBase::update(const QRect& rect)
{
    Q_UNUSED(rect)
    mWindow->update();
}

void QmlDialogWrapperBase::resize(int w, int h)
{
    mWindow->resize(QSize(w, h));
}

void QmlDialogWrapperBase::resize(const QSize &size)
{
    mWindow->resize(size);
}

QSize QmlDialogWrapperBase::size()
{
    return mWindow->size();
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
