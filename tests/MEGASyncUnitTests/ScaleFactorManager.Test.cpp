#include <catch.hpp>
#include "ScaleFactorManager.h"

constexpr auto scaleEnvirontmentVariableName{"QT_SCALE_FACTOR"};
constexpr auto scaleScreensEnvirontmentVariableName{"QT_SCREEN_SCALE_FACTORS"};
constexpr auto autoScaleEnvirontmentVariableName{"QT_AUTO_SCREEN_SCALE_FACTOR"};

void unsetEnvirontmentVariables()
{
    unsetenv(scaleEnvirontmentVariableName);
    unsetenv(scaleScreensEnvirontmentVariableName);
    unsetenv(autoScaleEnvirontmentVariableName);
}

SCENARIO("Scale factor calculation on linux platforms")
{
    unsetEnvirontmentVariables();

    GIVEN("A single screen with 1920x1080 resolution")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 1920, 1080, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is not set as scaling is not needed")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
                CHECK_FALSE(getenv(scaleScreensEnvirontmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 1920x1080 resolution and 200% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 1920, 1080, 192, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with the correct factor")
            {
                CHECK(getenv(scaleEnvirontmentVariableName) == std::string("1.66667"));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 3840, 2160, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with the correct factor")
            {
                CHECK(getenv(scaleEnvirontmentVariableName) == std::string("1.5"));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution and 200% screen scaled")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 3840, 2160, 192, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with the correct factor")
            {
                CHECK(getenv(scaleEnvirontmentVariableName) == std::string("2"));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution and display 200% scale with QT_AUTO_SCREEN_SCALE_FACTOR environtment variable set")
    {
        // When QT_AUTO_SCREEN_SCALE_FACTOR is set Qt treats the screan as 1920x1080
        // and sets the variable highDpiAutoScalingEnabled to 2.0
        // Qt return 96 dpi but xrdb return 192
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName", 1920, 1080, 192, 2.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with the correct factor")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
            }
        }
    }

    GIVEN("Two screens with 3840x2160 and 1920x1080 resolutions")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 96, 1.}, {"screenName2", 3840, 2160, 96, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with the correct factor")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
                CHECK(getenv(scaleScreensEnvirontmentVariableName) == std::string("1,000000;1,500000"));
            }
        }
    }

    GIVEN("Two screens with 3840x2160 and 1920x1080 resolutions scaled to 200%")
    {
        ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 192, 1.}, {"screenName2", 3840, 2160, 192, 1.}}, "Ubuntu20", "plasma");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with the correct factor")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
                CHECK(getenv(scaleScreensEnvirontmentVariableName) == std::string("1,666667;2,000000"));
            }
        }
    }
}

SCENARIO("Scale factor calculation on windows platforms")
{
    unsetEnvirontmentVariables();

    GIVEN("A single screen set with 1920x1080 resolution with 1.0 device pixel ratio")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 1920, 1080, 96, 1.0}}, "win64", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is not set as scaling is not needed")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
                CHECK_FALSE(getenv(scaleScreensEnvirontmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 1920x1080 resolution with 2.0 device pixel ratio")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 1920, 1080, 96, 2.0}}, "win64", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set to the maximum value that fits the available screen")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
            }
        }
    }

    GIVEN("A single screen with 3840x2160 resolution with 3.1 device pixel ratio")
    {
        ScaleFactorManager scaleFactorManager(OsType::WIN, {{"screenName", 3840, 2160, 96, 2.0}}, "win64", "");

        WHEN("Scale factor is set")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is set with maximum factor ratio allowed")
            {
                CHECK_FALSE(getenv(scaleEnvirontmentVariableName));
            }
        }
    }
}

SCENARIO("Retrieve loggin after calculations")
{
    unsetEnvirontmentVariables();

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

    GIVEN("A two screen setup with QT_AUTO_SCREEN_SCALE_FACTOR environtment set")
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

SCENARIO("Environtment variables already set before execution")
{
    unsetEnvirontmentVariables();
    ScaleFactorManager scaleFactorManager(OsType::LINUX, {{"screenName1", 1920, 1080, 192, 1.}, {"screenName2", 3840, 2160, 192, 1.}}, "Ubuntu20", "plasma");

    GIVEN("QT_SCALE_FACTOR environtment variable already set")
    {
        setenv("QT_SCALE_FACTOR", "1.27", true);

        WHEN("Set scale factor is executed")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is left with the value already set")
            {
                CHECK(getenv(scaleEnvirontmentVariableName) == std::string("1.27"));

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

    GIVEN("QT_SCREEN_SCALE_FACTORS environtment variable already set")
    {
        const auto variableValue{"screenName1=1.22;screenName2=1.21"};
        setenv("QT_SCREEN_SCALE_FACTORS", variableValue, true);

        WHEN("Set scale factor is executed")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("Environtment variable is left with the value already set")
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

    GIVEN("QT_SCREEN_SCALE_FACTORS environtment variable already set but screen names does not match")
    {
        const auto variableValue{"wrong_screen_name=1.22;screenName2=1.21"};
        setenv("QT_SCREEN_SCALE_FACTORS", variableValue, true);

        WHEN("Set scale factor is executed")
        {
            scaleFactorManager.setScaleFactorEnvironmentVariable();

            THEN("QT_SCALE_FACTOR environtment variable is set with the calculated scale")
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
