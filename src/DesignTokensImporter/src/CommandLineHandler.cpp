#include "CommandLineHandler.h"

#include <QCoreApplication>
#include <QDebug>

#include <mutex>

namespace
{
std::once_flag parserInitializedFlag;
QCommandLineOption megaSyncPathOption(QStringList() << "m"
                                                    << "megasync-path",
                                      "Path to MEGASync directory",
                                      "path");
QCommandLineOption tokensFileOption(QStringList() << "t"
                                                  << "tokens-file",
                                    "Path to design tokens file",
                                    "path");
}

namespace DTI
{
void CommandLineHandler::setupCommandLineOptions()
{
    mCommandParser.setApplicationDescription("DesignTokensImporter");
    mCommandParser.addHelpOption();
    mCommandParser.addOption(megaSyncPathOption);
    mCommandParser.addOption(tokensFileOption);
}

void CommandLineHandler::parseAndValidateCommandArgs()
{
    std::call_once(parserInitializedFlag, setupCommandLineOptions);
    mCommandParser.process(QCoreApplication::arguments());
    if (!mCommandParser.isSet(megaSyncPathOption))
    {
        qCritical() << "Error: --megasync-path (-m) option is required.";
        mCommandParser.showHelp(EXIT_FAILURE);
    }

    if (!mCommandParser.isSet(tokensFileOption))
    {
        qCritical() << "Error: --tokens-file (-t) option is required.";
        mCommandParser.showHelp(EXIT_FAILURE);
    }
}

QString CommandLineHandler::getMEGASyncPath()
{
    return mCommandParser.value(megaSyncPathOption);
}

QString CommandLineHandler::getTokenFilePath()
{
    return mCommandParser.value(tokensFileOption);
}
}
