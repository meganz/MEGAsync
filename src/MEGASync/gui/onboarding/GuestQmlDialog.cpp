#include "GuestQmlDialog.h"

#include "Platform.h"

GuestQmlDialog::GuestQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
    setFlags(flags() | Qt::FramelessWindowHint);
    mHideTimer.setSingleShot(true);

    QObject::connect(&mHideTimer, &QTimer::timeout, this, [this](){
        this->hide();
    });

    QObject::connect(this, &GuestQmlDialog::activeChanged, [=]() {
        if (!this->isActive()) {
            mHideTimer.start(200);
        }
    });
}

GuestQmlDialog::~GuestQmlDialog()
{
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
