#include "GuestWindow.h"

#include "Platform.h"

GuestWindow::GuestWindow(QWindow *parent)
    : QQuickWindow(parent)
{
}

GuestWindow::~GuestWindow()
{
}

void GuestWindow::realocate()
{
    int posx, posy;
    Platform::getInstance()->calculateInfoDialogCoordinates(geometry(), &posx, &posy);
    setX(posx);
    setY(posy);
}
