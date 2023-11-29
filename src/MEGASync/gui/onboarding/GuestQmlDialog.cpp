#include "GuestQmlDialog.h"

#include "Platform.h"

GuestQmlDialog::GuestQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
    setFlags(flags() | Qt::FramelessWindowHint);

    QObject::connect(this, &GuestQmlDialog::activeChanged, [=]() {
        if (!this->isActive()) {
            this->hide();
        }
    });
}

GuestQmlDialog::~GuestQmlDialog()
{
}

bool GuestQmlDialog::isHiddenForLongTime() const
{
    return QDateTime::currentMSecsSinceEpoch() - mLastHideTime > 500;
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
}

void GuestQmlDialog::hideEvent(QHideEvent *event)
{
    mLastHideTime = QDateTime::currentMSecsSinceEpoch();
    QmlDialog::hideEvent(event);
}
