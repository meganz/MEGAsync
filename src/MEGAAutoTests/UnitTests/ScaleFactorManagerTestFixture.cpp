#include "ScaleFactorManagerTestFixture.h"

#include <catch.hpp>

ScaleFactorManagerTests::ScaleFactorManagerTests()
#if defined(_WIN32) || defined(__APPLE__)
    :
    mDecimalSeparator(".")
#else
    :
    mDecimalSeparator(",")
#endif
{}

void ScaleFactorManagerTests::testScaleFactorWithoutScaling(const std::string& description,
                                                            const ScreensInfo& screens,
                                                            const char* endDescription)
{
    SECTION(createSectionName(description, endDescription, "No scaling needed"))
    {
        ScaleFactorManager scaleFactorManager(mOsType, screens, mOsName, mDesktopName);
        scaleFactorManager.setScaleFactorEnvironmentVariable();
        CHECK_FALSE(getenv(scaleEnvironmentVariableName));
        CHECK_FALSE(getenv(scaleScreensEnvironmentVariableName));
    }
}

void ScaleFactorManagerTests::testScaleFactorWithScaling(const std::string& description,
                                                         const ScreensInfo& screens,
                                                         const std::string& expectedScaling,
                                                         const char* endDescription)
{
    SECTION(createSectionName(description, endDescription, "Envvar set correctly"))
    {
        ScaleFactorManager scaleFactorManager(mOsType, screens, mOsName, mDesktopName);
        scaleFactorManager.setScaleFactorEnvironmentVariable();
        CHECK(getenv(scaleEnvironmentVariableName) == expectedScaling);
    }
}

void ScaleFactorManagerTests::testTwoScreensScaleFactorScaling(const std::string& description,
                                                               const ScreensInfo& screens,
                                                               const std::string& expectedScaling,
                                                               const char* endDescription)
{
    SECTION(createSectionName(description, endDescription, "Envvar set correctly"))
    {
        ScaleFactorManager scaleFactorManager(mOsType, screens, mOsName, mDesktopName);
        scaleFactorManager.setScaleFactorEnvironmentVariable();
        CHECK_FALSE(getenv(scaleEnvironmentVariableName));
        CHECK(getenv(scaleScreensEnvironmentVariableName) == expectedScaling);
    }
}

void ScaleFactorManagerTests::testGetLogMessages()
{
    const double dpi = 96;
    ScaleFactorManager scaleFactorManager(mOsType,
                                          createFullHDand4Kdesk(dpi),
                                          mOsName,
                                          mDesktopName);
    scaleFactorManager.setScaleFactorEnvironmentVariable();

    checkLogMessages(scaleFactorManager, dpi, "QT_SCREEN_SCALE_FACTORS set to 1;1.5");
}

void ScaleFactorManagerTests::testSetScaleFactorWhenEnvVarAlreadySet(
    const std::string& expectedLastLogLine)
{
    std::vector<std::string> lastLines = {expectedLastLogLine};
    testSetScaleFactorWhenEnvVarAlreadySet(lastLines);
}

void ScaleFactorManagerTests::testSetScaleFactorWhenEnvVarAlreadySet(
    const std::vector<std::string>& expectedLastLogLines)
{
    const double dpi = 192;
    ScaleFactorManager scaleFactorManager(mOsType,
                                          createFullHDand4Kdesk(dpi),
                                          mOsName,
                                          mDesktopName);
    scaleFactorManager.setScaleFactorEnvironmentVariable();
    checkLogMessages(scaleFactorManager, dpi, expectedLastLogLines);
}

void ScaleFactorManagerTests::checkLogMessages(ScaleFactorManager& manager,
                                               double dpi,
                                               const std::string& lastLogLine)
{
    std::vector<std::string> lastLines = {lastLogLine};
    checkLogMessages(manager, dpi, lastLines);
}

void ScaleFactorManagerTests::checkLogMessages(ScaleFactorManager& manager,
                                               double dpi,
                                               const std::vector<std::string>& lastLogLines)
{
    std::vector<std::string> expectedLogMessages{"Ubuntu20 (plasma)"};
    auto dpiStr = std::to_string(static_cast<int>(dpi));
    expectedLogMessages.emplace_back("Screen detected: screenName1, 1920, 1080, " + dpiStr + ", 1");
    expectedLogMessages.emplace_back("Screen detected: screenName2, 3840, 2160, " + dpiStr + ", 1");
    for (auto additionalLine: lastLogLines)
    {
        expectedLogMessages.emplace_back(additionalLine);
    }

    CHECK(toStdStringVector(manager.getLogMessages()) == expectedLogMessages);
}

ScreensInfo ScaleFactorManagerTests::create640x480desk()
{
    return {{"screenName", 640, 480, 96, 1.0}};
}

ScreensInfo ScaleFactorManagerTests::createFullHDdesk(double dpi, double ratio)
{
    return {{"screenName", 1920, 1080, dpi, ratio}};
}

ScreensInfo ScaleFactorManagerTests::create4Kdesk(double dpi, double ratio)
{
    return {{"screenName", 3840, 2160, dpi, ratio}};
}

ScreensInfo ScaleFactorManagerTests::createFullHDand4Kdesk(double dpi, double ratio)
{
    return {{"screenName1", 1920, 1080, dpi, ratio}, {"screenName2", 3840, 2160, dpi, ratio}};
}

std::string ScaleFactorManagerTests::floatVal(const std::string& val, const std::string& decimals)
{
    return val + mDecimalSeparator + decimals;
}

#ifdef WIN32
void ScaleFactorManagerTests::unsetenv(std::string name)
{
    _putenv((name + "=").c_str());
}

void ScaleFactorManagerTests::setenv(std::string name, std::string value, int /*overwrite*/)
{
    _putenv((name + "=" + value).c_str());
}
#endif

void ScaleFactorManagerTests::unsetEnvironmentVariables()
{
    unsetenv(scaleEnvironmentVariableName);
    unsetenv(scaleScreensEnvironmentVariableName);
    unsetenv(autoScaleEnvironmentVariableName);
}

void ScaleFactorManagerTests::configureAsUbuntu20()
{
    mOsType = OsType::LINUX;
    mOsName = "Ubuntu20";
    mDesktopName = "plasma";
}

void ScaleFactorManagerTests::configureAsDeepin20()
{
    mOsType = OsType::LINUX;
    mOsName = "Deepin 20";
    mDesktopName = "DDE";
}

void ScaleFactorManagerTests::configureAsWindows10()
{
    mOsType = OsType::WIN;
    mOsName = "Windows 10";
    mDesktopName = "";
}

std::string ScaleFactorManagerTests::createSectionName(const std::string& description,
                                                       const char* endDescription,
                                                       const char* defaultEndDescription)
{
    const char* expectedEndDescription = endDescription ? endDescription : defaultEndDescription;
    return description + " - " + expectedEndDescription;
}

std::vector<std::string> ScaleFactorManagerTests::toStdStringVector(const QVector<QString>& param)
{
    std::vector<std::string> result;
    for (const auto& s: param)
    {
        result.push_back(s.toUtf8().constData());
    }
    return result;
}
