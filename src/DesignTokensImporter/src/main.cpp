#include "CommandLineHandler.h"
#include "DesignTokensImporter.h"
#include "Utilities.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

using namespace DTI;

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    CommandLineHandler::parseAndValidateCommandArgs();

    const QString megaSyncPath = CommandLineHandler::getMEGASyncPath();
    const QString tokensFilePath = CommandLineHandler::getTokenFilePath();

    if (!DesignTokensImporter::initialize(megaSyncPath, tokensFilePath))
    {
        Utilities::logInfoMessage(
            "Failed to initialize DesignTokensImporter. Please check the paths.");

        return EXIT_FAILURE;
    }
    DesignTokensImporter::run();

    Utilities::logInfoMessage("DesignTokensImporter has finished.");

    // Stop the event loop and exit the application
    QCoreApplication::quit();

    return 0;
}
