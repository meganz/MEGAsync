#include <catch.hpp>
#include "ScaleFactorManager.h"
#include <string>

constexpr auto scaleEnvironmentVariableName{"QT_SCALE_FACTOR"};
constexpr auto scaleScreensEnvironmentVariableName{"QT_SCREEN_SCALE_FACTORS"};
constexpr auto autoScaleEnvironmentVariableName{"QT_AUTO_SCREEN_SCALE_FACTOR"};


#ifdef WIN32
void unsetenv(std::string name)
{
    _putenv((name + "=").c_str());
}
void setenv(std::string name, std::string value, int /*overwrite*/)
{
    _putenv((name + "=" + value).c_str());
}
#endif

void unsetEnvironmentVariables()
{
    unsetenv(scaleEnvironmentVariableName);
    unsetenv(scaleScreensEnvironmentVariableName);
    unsetenv(autoScaleEnvironmentVariableName);
}

SCENARIO("Scale factor calculation on linux platforms")
{
    unsetEnvironmentVariables();

    GIVEN("A single screen with 1920x1080 resolution")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 1920, 1080, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is not set as scaling is not needed")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
                CHECK_FALSE(getenv(scaleScreensEnvironmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 1920x1080 resolution and 200% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 1920, 1080, 192, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("environment variable is set with the correct factor")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("1.66667"));
            }
        }
    }

    GIVEN("A single screen with 640x480 resolution and 100% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 640, 480, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("No need to rescale because the scale calculated is lower than 1.0 and that is the minimum value allowed")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
                CHECK_FALSE(getenv(scaleScreensEnvironmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 8000x6000 resolution and 100% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 8000, 6000, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set to the maximum value allowed")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("3"));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 3840, 2160, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set with the correct factor")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("1.5"));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution and 200% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 3840, 2160, 192, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set with the correct factor")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("2"));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution and display 200% scale with QT_AUTO_SCREEN_SCALE_FACTOR Environment variable set")
    {
        // When QT_AUTO_SCREEN_SCALE_FACTOR is set Qt treats the screan as 1920x1080
        // and sets the variable highDpiAutoScalingEnabled to 2.0
        // Qt return 96 dpi but xrdb return 192
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 1920, 1080, 192, 2.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Scale factor environment variable is not set because highDpi is enabled")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
            }
        }
    }

    GIVEN("Two screens with 3840x2160 and 1920x1080 resolutions")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 96, 1.}, {"screenName2", 3840, 2160, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set with the correct factor")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
                CHECK(getenv(scaleScreensEnvironmentVariableName) == std::string("1,000000;1,500000"));
            }
        }
    }

    GIVEN("Two screens with 3840x2160 and 1920x1080 resolutions scaled to 200%")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 192, 1.}, {"screenName2", 3840, 2160, 192, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set with the correct factor")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
                CHECK(getenv(scaleScreensEnvironmentVariableName) == std::string("1,666667;2,000000"));
            }
        }
    }

    GIVEN("Two screens with 3840x2160 and 1920x1080 resolutions scaled to 200% on Deepin 20 distro")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 3840, 2160, 192, 1.}, {"screenName2", 1920, 1080, 192, 1.}}, "Deepin 20", "DDE");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set for all screens with the minimum factor calculated for Deepin distro")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("1.66667"));
            }
        }
    }
}

SCENARIO("Scale factor calculation on windows platforms")
{
    unsetEnvironmentVariables();

    GIVEN("A single screen set with 1920x1080 resolution with 100% scale")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 1920, 1040, 96., 1.}}, "Windows 10", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is not set as scaling is not needed")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
                CHECK_FALSE(getenv(scaleScreensEnvironmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 1920x1080 resolution with 150% scale")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 960, 510, 72., 2.0}}, "Windows 10", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is set with the correct factor since the biggest MEGASync window does not fit the available screen space.")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("0.75"));
            }
        }
    }

    GIVEN("A single screen with 640x480 resolution and 100% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 640, 480, 96, 1.}}, "Windows 10", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("No need to rescale because the scale calculated is lower than 1.0 and that is the minimum value allowed when hidpi is disabled")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
                CHECK_FALSE(getenv(scaleScreensEnvironmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution with 150% scale")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 1920, 1050, 72., 2.0}}, "Windows 10", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Scale factor environment variable is not set because highDpi is enabled and biggest window fits the available screen")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution with 200% scale")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 1920, 1050, 96., 2.0}}, "Windows 10", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Scale factor environment variable is not set because highDpi is enabled and bigger window fits the available screen")
            {
                CHECK_FALSE(getenv(scaleEnvironmentVariableName));
            }
        }
    }
}

SCENARIO("Retrieve loggin after calculations")
{
    unsetEnvironmentVariables();

    GIVEN("A two screen setup")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 96, 1.}, {"screenName2", 3840, 2160, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Loged messages can be retrieved")
            {
                const auto logMessages{scaleFactorManager.getLogMessages()};
                std::vector<std::string> expectedLogMessages{"Ubuntu20 (plasma)"};
                expectedLogMessages.emplace_back("Screen detected: screenName1, 1920, 1080, 96,000000, 1,000000");
                expectedLogMessages.emplace_back("Screen detected: screenName2, 3840, 2160, 96,000000, 1,000000");
                expectedLogMessages.emplace_back("QT_SCREEN_SCALE_FACTORS set to 1,000000;1,500000");

                CHECK(logMessages == expectedLogMessages);
            }
        }
    }

    GIVEN("A two screen setup with QT_AUTO_SCREEN_SCALE_FACTOR environment set")
    {
        setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1", true);
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 96, 1.}, {"screenName2", 3840, 2160, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Loged messages can be retrieved")
            {
                const auto logMessages{scaleFactorManager.getLogMessages()};
                std::vector<std::string> expectedLogMessages{"Ubuntu20 (plasma)"};
                expectedLogMessages.emplace_back("Screen detected: screenName1, 1920, 1080, 96,000000, 1,000000");
                expectedLogMessages.emplace_back("Screen detected: screenName2, 3840, 2160, 96,000000, 1,000000");
                expectedLogMessages.emplace_back("QT_SCREEN_SCALE_FACTORS set to 1,000000;1,500000");

                CHECK(logMessages == expectedLogMessages);
            }
        }
    }
}

SCENARIO("Environment variables already set before execution")
{
    unsetEnvironmentVariables();
    ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 192, 1.}, {"screenName2", 3840, 2160, 192, 1.}}, "Ubuntu20", "plasma");

    GIVEN("QT_SCALE_FACTOR Environment variable already set")
    {
        setenv("QT_SCALE_FACTOR", "1.27", true);

        WHEN("Set scale factor is executed")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is left with the value already set")
            {
                CHECK(getenv(scaleEnvironmentVariableName) == std::string("1.27"));

                AND_THEN("Logs can be retrieved")
                {
                    std::vector<std::string> expectedLogMessages{"Ubuntu20 (plasma)"};
                    expectedLogMessages.emplace_back("Screen detected: screenName1, 1920, 1080, 192,000000, 1,000000");
                    expectedLogMessages.emplace_back("Screen detected: screenName2, 3840, 2160, 192,000000, 1,000000");
                    expectedLogMessages.emplace_back("Scale factor not calculated because QT_SCALE_FACTOR is already set to: 1.27");

                    const auto logMessages{scaleFactorManager.getLogMessages()};
                    CHECK(logMessages == expectedLogMessages);
                }
            }
        }
    }

    GIVEN("QT_SCREEN_SCALE_FACTORS environment variable already set")
    {
        const auto variableValue{"screenName1=1.22;screenName2=1.21"};
        setenv("QT_SCREEN_SCALE_FACTORS", variableValue, true);

        WHEN("Set scale factor is executed")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environment variable is left with the value already set")
            {
                CHECK(getenv("QT_SCREEN_SCALE_FACTORS") == std::string(variableValue));
                CHECK_FALSE(getenv("QT_SCALE_FACTOR"));

                AND_THEN("Logs can be retrieved")
                {
                    std::vector<std::string> expectedLogMessages{"Ubuntu20 (plasma)"};
                    expectedLogMessages.emplace_back("Screen detected: screenName1, 1920, 1080, 192,000000, 1,000000");
                    expectedLogMessages.emplace_back("Screen detected: screenName2, 3840, 2160, 192,000000, 1,000000");
                    expectedLogMessages.emplace_back("Scale factor not calculated because QT_SCREEN_SCALE_FACTORS is already set to: "
                                                     + std::string(variableValue));

                    const auto logMessages{scaleFactorManager.getLogMessages()};
                    CHECK(logMessages == expectedLogMessages);
                }
            }
        }
    }

    GIVEN("QT_SCREEN_SCALE_FACTORS environment variable already set but screen names does not match")
    {
        const auto variableValue{"wrong_screen_name=1.22;screenName2=1.21"};
        setenv("QT_SCREEN_SCALE_FACTORS", variableValue, true);

        WHEN("Set scale factor is executed")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("QT_SCALE_FACTOR environment variable is set with the calculated scale")
            {
                AND_THEN("Logs can be retrieved")
                {
                    std::vector<std::string> expectedLogMessages{"Ubuntu20 (plasma)"};
                    expectedLogMessages.emplace_back("Screen detected: screenName1, 1920, 1080, 192,000000, 1,000000");
                    expectedLogMessages.emplace_back("Screen detected: screenName2, 3840, 2160, 192,000000, 1,000000");
                    expectedLogMessages.emplace_back("Screen name screenName1 not found in predefined QT_SCREEN_SCALE_FACTORS: "
                                                     + std::string(variableValue));
                    expectedLogMessages.emplace_back("QT_SCREEN_SCALE_FACTORS set to 1,666667;2,000000");

                    const auto logMessages{scaleFactorManager.getLogMessages()};
                    CHECK(logMessages == expectedLogMessages);
                }
            }
        }
    }
}
