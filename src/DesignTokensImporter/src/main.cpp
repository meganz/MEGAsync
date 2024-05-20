#include "DesignTokensImporter.h"

#include "PathProvider.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

using namespace DTI;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Point to MEGASync as current working directory
    QDir::setCurrent(PathProvider::RELATIVE_MEGASYNC_PATH);

    qDebug() << "Start parsing process";

    DesignTokensImporter::run();

    qDebug() << "Finished";

    // Stop the event loop and exit the application
    QCoreApplication::quit();

    return 0;
}
