#include "megadebugserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MegaDebugServer w;
    w.show();

    return a.exec();
}
