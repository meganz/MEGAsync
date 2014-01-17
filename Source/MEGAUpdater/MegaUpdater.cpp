#include "MegaUpdater.h"
#include "UpdaterGUI.h"

int main(int argc, char *argv[])
{
    MegaUpdater app(argc, argv);
    UpdaterGUI gui;
    gui.show();
    return app.exec();
}

MegaUpdater::MegaUpdater(int &argc, char **argv) :
    QApplication(argc, argv)
{
    setOrganizationName(QString::fromAscii("Mega Limited"));
    setOrganizationDomain(QString::fromAscii("mega.co.nz"));
    setApplicationName(QString::fromAscii("MEGAupdater"));
}
