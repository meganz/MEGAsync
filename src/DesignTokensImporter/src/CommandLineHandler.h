#ifndef COMMANDLINE_HANDLER_H
#define COMMANDLINE_HANDLER_H

#include <QCommandLineParser>

namespace DTI
{
class CommandLineHandler
{
public:
    static void parseAndValidateCommandArgs();
    static QString getMEGASyncPath();
    static QString getTokenFilePath();

private:
    static void setupCommandLineOptions();
    static inline QCommandLineParser mCommandParser;
};
}
#endif // COMMANDLINE_HANDLER_H
