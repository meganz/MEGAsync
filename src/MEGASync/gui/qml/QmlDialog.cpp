#include "QmlDialog.h"

#include <QEvent>
#include <QScreen>

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

void QmlDialog::centerAndRaise()
{
    // The following four lines are required by Ubuntu to bring the window to the front and
    // move it to the center of the current screen, if the screen is a part of a virtual desktop or
    // multiple screen we will need add the current screen offset(topleft) to the calculated central
    // position.
    const auto& geometry(QmlDialog::screen()->geometry());
    int xPos(geometry.x() + static_cast<int>(geometry.width() * 0.5 - width() * 0.5));
    int yPos(geometry.y() + static_cast<int>(geometry.height() * 0.5 - height() * 0.5));

    hide();
    QmlDialog::setPosition(xPos, yPos);
    show();

    // The following two lines are required by Windows (activate) and macOS (raise)
    QmlDialog::requestActivate();
    QmlDialog::raise();
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
