#include "QmlDialog.h"

#include <QEvent>

namespace
{
constexpr const char* DEFAULT_RES_MEGA_ICON(":/images/app_ico.ico");
constexpr const char* DEFAULT_TITLE("MEGA");
}

QmlDialog::QmlDialog(QWindow* parent):
    QQuickWindow(parent),
    mIconSrc(QString::fromUtf8(DEFAULT_RES_MEGA_ICON)),
    mInstancesManager(new QmlInstancesManager())
{
    setFlags(flags() | Qt::Dialog);

    setTitle(QString::fromUtf8(DEFAULT_TITLE));

    connect(this,
            &QmlDialog::requestPageFocus,
            this,
            &QmlDialog::onRequestPageFocus,
            Qt::QueuedConnection);

    connect(mInstancesManager,
            &QmlInstancesManager::instancesChanged,
            this,
            &QmlDialog::instancesManagerChanged);
}

void QmlDialog::setIconSrc(const QString& iconSrc)
{
    QString source = iconSrc;
    if (iconSrc.startsWith(QString::fromUtf8("qrc:")))
    {
        source = source.mid(3);
    }

    if (source != mIconSrc)
    {
        mIconSrc = source;
        setIcon(QIcon(mIconSrc));
    }
}

QmlInstancesManager* QmlDialog::getInstancesManager()
{
    return mInstancesManager;
}

bool QmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close)
    {
        emit finished();
    }

    return QQuickWindow::event(event);
}

void QmlDialog::onRequestPageFocus()
{
    emit initializePageFocus();
}
