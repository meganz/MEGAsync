#include "GuestQmlDialog.h"

#include "Platform.h"

#include <QGuiApplication>

namespace
{
bool isNativeWaylandSession()
{
    return QGuiApplication::platformName().contains(QString::fromUtf8("wayland"),
                                                    Qt::CaseInsensitive);
}
}

GuestQmlDialog::GuestQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
    setFlags(flags() | Qt::FramelessWindowHint);

    QObject::connect(this, &GuestQmlDialog::activeChanged, [=]() {
        // Native Wayland focus transitions from tray hosts are unreliable here;
        // treating every inactive transition as a dismissal makes the popup unusable.
        if (isNativeWaylandSession() && !this->isActive())
        {
            return;
        }

        emit guestActiveChanged(this->isActive());
    });
}

bool GuestQmlDialog::isHiddenForLongTime() const
{
    return !isVisible() && QDateTime::currentMSecsSinceEpoch() - mLastHideTime > 500;
}

void GuestQmlDialog::realocate()
{
    int posx, posy;
    Platform::getInstance()->calculateInfoDialogCoordinates(geometry(), &posx, &posy);
    setX(posx);
    setY(posy);
}

void GuestQmlDialog::showEvent(QShowEvent *event)
{
    realocate();
    QmlDialog::showEvent(event);

    emit initializePageFocus();
}

void GuestQmlDialog::hideEvent(QHideEvent *event)
{
    mLastHideTime = QDateTime::currentMSecsSinceEpoch();
    QmlDialog::hideEvent(event);

    emit hideRequested();
}
