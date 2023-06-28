#include "GuestController.h"
#include "MegaApplication.h"
#include "DialogOpener.h"
#include "onboarding/Onboarding.h"

#include <QDebug>

GuestController::GuestController(QObject *parent)
    : QObject(parent)
{
}

void GuestController::onAboutMEGAClicked()
{
    MegaSyncApp->onAboutClicked();
}

void GuestController::onPreferencesClicked()
{
    MegaSyncApp->openSettings();
}

void GuestController::onExitClicked()
{
    MegaSyncApp->tryExitApplication();
}
