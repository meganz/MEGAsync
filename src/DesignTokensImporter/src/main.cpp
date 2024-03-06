#include "TokenManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

static const QString RELATIVE_MEGASYNC_PATH = QString::fromLatin1("../../MEGASync");

using namespace DTI;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Point to MEGASync as current working directory
    QDir::setCurrent(RELATIVE_MEGASYNC_PATH);

    qDebug() << "Start parsing process";

    TokenManager::instance()->run();

    qDebug() << "Finished";

    // Stop the event loop and exit the application
    QCoreApplication::quit();

    return 0;
}
