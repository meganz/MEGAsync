#include "catch/catch.hpp"
#include "ScaleFactorManagerTestFixture.h"

#include <string>

TEST_CASE_METHOD(ScaleFactorManagerTests, "setScaleFactorEnvironmentVariable()")
{
    unsetEnvironmentVariables();
    SECTION("Linux - Ubuntu 20")
    {
        configureAsUbuntu20();

        testScaleFactorWithoutScaling("Single screen at FullHD", createFullHDdesk(96));

        // Failing since the beginning
        // testScaleFactorWithScaling("Single screen at FullHD and 200% scaling",
        // createFullHDdesk(192), "1.66667");

        testScaleFactorWithoutScaling("Single screen at 640x480 and 100% scaling",
                                      create640x480desk(),
                                      "No scaling needed, scale lower than 1.0");

        // Failing since the beginning
        // testScaleFactorWithScaling("Single screen at 8000x6000 and 100% scaling",
        // createFullHDdesk(192),
        //                           "1.66667", "Scaling set to max");

        testScaleFactorWithScaling("Single screen at 4K", create4Kdesk(96), "1.5");

        testScaleFactorWithScaling("Single screen at 4K and 200% scaling", create4Kdesk(192), "2");

        // When QT_AUTO_SCREEN_SCALE_FACTOR is set Qt treats the screan as 1920x1080
        // and sets the variable highDpiAutoScalingEnabled to 2.0
        // Qt return 96 dpi but xrdb return 192
        testScaleFactorWithoutScaling(
            "Single screen at 4K and 200% scaling with QT_AUTO_SCREEN_SCALE_FACTOR",
            create4Kdesk(192, 2.),
            "Scale factor environment variable is not set because highDpi is enabled");

        testTwoScreensScaleFactorScaling("Two screens at 4K and FullHD",
                                         createFullHDand4Kdesk(96),
                                         "1;1.5");

        // Failing since the beginning
        // testTwoScreensScaleFactorScaling("Two screens at 4K and FullHD with 200% scaling",
        //                            createFullHDand4Kdesk(192), "1,666667;2,000000");

        SECTION("QT_SCALE_FACTOR Environment variable already set")
        {
            // Needs to be auto to comply with setenv() on Linux and Windows
            // (const char* on Linux, std::string on Windows)
            const auto envvarValue = "1.27";
            setenv("QT_SCALE_FACTOR", envvarValue, true);

            testSetScaleFactorWhenEnvVarAlreadySet(
                "Scale factor not calculated because QT_SCALE_FACTOR is already set to: " +
                std::string(envvarValue));

            CHECK(getenv(scaleEnvironmentVariableName) == std::string(envvarValue));
        }

        SECTION("QT_SCREEN_SCALE_FACTORS environment variable already set")
        {
            // Needs to be auto to comply with setenv() on Linux and Windows
            // (const char* on Linux, std::string on Windows)
            const auto envvarValue = "screenName1=1.22;screenName2=1.21";
            setenv("QT_SCREEN_SCALE_FACTORS", envvarValue, true);

            testSetScaleFactorWhenEnvVarAlreadySet(
                "Scale factor not calculated because QT_SCREEN_SCALE_FACTORS is already set to: " +
                std::string(envvarValue));

            CHECK(getenv("QT_SCREEN_SCALE_FACTORS") == std::string(envvarValue));
            CHECK_FALSE(getenv("QT_SCALE_FACTOR"));
        }

        // Failing since the beginning
        /*
        SECTION("QT_SCREEN_SCALE_FACTORS environment variable already set but screen names does not
        match")
        {
            // Needs to be auto to comply with setenv() on Linux and Windows
            // (const char* on Linux, std::string on Windows)
            const auto envvarValue = "wrong_screen_name=1.22;screenName2=1.21";
            setenv("QT_SCREEN_SCALE_FACTORS", envvarValue, true);

            std::vector<std::string> lastLines = {
                "Screen name screenName1 not found in predefined QT_SCREEN_SCALE_FACTORS: " +
        std::string(envvarValue), "QT_SCREEN_SCALE_FACTORS set to 1,666667;2,000000"
            };
            testSetScaleFactorWhenEnvVarAlreadySet(lastLines);
        }*/
    }

    SECTION("Linux - Deepin 20")
    {
        configureAsDeepin20();

        // Failing since the beginning
        // testScaleFactorWithScaling("Two screens at 4K and FullHD with 200% scaling on Deepin 20
        // distro",
        //                              createFullHDand4Kdesk(192), "1.66667");
    }

    SECTION("Windows 10")
    {
        configureAsWindows10();

        testScaleFactorWithoutScaling("Single screen at FullHD and 100% scaling",
                                      createFullHDdesk(96));

        // Failing since the beginning
        // testScaleFactorWithScaling("Single screen at 960x510 and 200% scaling", {{"screenName",
        // 960, 510, 72., 2.0}},
        //                              "0.75");

        testScaleFactorWithoutScaling("Single screen at 640x480 and 100% scaling",
                                      create640x480desk());
    }
}

TEST_CASE_METHOD(ScaleFactorManagerTests, "getLogMessages()")
{
    unsetEnvironmentVariables();
    configureAsUbuntu20();

    SECTION("Two screen setup")
    {
        testGetLogMessages();
    }

    SECTION("Two screen setup with QT_AUTO_SCREEN_SCALE_FACTOR environment set")
    {
        setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1", true);
        testGetLogMessages();
    }
}
