#include "AllMegaIncludes.h"

#define CATCH_CONFIG_RUNNER
#include "trompeloeil.hpp"
#include <catch.hpp>

/******************************
 *
 * Note about Unit Tests :
 *
 * Add "-r mega" to the command line parameters while executing.
 * This uses our in-house reporter, which has a much better output.
 * You can change the verbosity of the output using the additional "-v "
 * parameter and use one of the following values :
 * - quiet : Only display the final results for each test case
 * - normal (default) : Display the final results for each test case and the details about failures
 * - high : Display  the test cases and each section and subsection, plus the details about failures
 *
 * You can execute it in QtCreator's console, but the display is better in a dedicated terminal.
 *
 *****************************/

void qtSilencedHandler(QtMsgType type, const QMessageLogContext&, const QString&)
{
    if (type == QtFatalMsg)
    {
        abort();
    }
}

int main(int argc, char* argv[])
{
    qInstallMessageHandler(qtSilencedHandler);

    MegaApplication app(argc, argv);

    trompeloeil::set_reporter(
        [](trompeloeil::severity s, const char* file, unsigned long line, std::string const& msg)
        {
            std::ostringstream os;
            if (line)
                os << file << ':' << line << '\n';
            os << msg;
            auto failure = os.str();
            if (s == trompeloeil::severity::fatal)
            {
                FAIL(failure);
            }
            else
            {
                CAPTURE(failure);
                CHECK(failure.empty());
            }
        });

    // What makes Test Results tab active in Qt Creator / Linux
    // is a QTest::exec() call. And only its results are considered..

    return std::min(Catch::Session().run(argc, argv), 0xff);
}
