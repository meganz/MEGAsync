#include "GuestQmlDialog.h"

#include "Platform.h"

GuestQmlDialog::GuestQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
    QObject::connect(this, &GuestQmlDialog::activeChanged, [=]() {
        if (!this->isActive()) {
            this->hide();
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
