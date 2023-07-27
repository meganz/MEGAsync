#include "GuestWindow.h"

#include "Platform.h"

GuestWindow::GuestWindow(QWindow *parent)
    : QmlDialog(parent)
{
    qDebug() << "GuestWindow constructor";
    QObject::connect(this, &GuestWindow::activeChanged, [=]() {
        if (!this->isActive()) {
            this->hide();
            qDebug() << "hide";
        }
    });
}

GuestWindow::~GuestWindow()
{
    qDebug() << "destructor";
}

void GuestWindow::realocate()
{
    int posx, posy;
    Platform::getInstance()->calculateInfoDialogCoordinates(geometry(), &posx, &posy);
    setX(posx);
    setY(posy);
}
