#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QProgressBar>
#include "megaapi.h"

namespace GuiUtilities
{
    void updateDataRequestProgressBar(QProgressBar* progressBar, mega::MegaRequest *request);
}

#endif // GUIUTILITIES_H
