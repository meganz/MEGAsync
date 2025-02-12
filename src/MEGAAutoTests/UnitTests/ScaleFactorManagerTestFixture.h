#ifndef SCALEFACTORMANAGERTESTFIXTURE_H
#define SCALEFACTORMANAGERTESTFIXTURE_H

#include "ScaleFactorManager.h"

#include <string>

class ScaleFactorManagerTests
{
public:
    ScaleFactorManagerTests();

protected:
    void testScaleFactorWithoutScaling(const std::string& description,
                                       const ScreensInfo& screens,
                                       const char* endDescription = nullptr);

    void testScaleFactorWithScaling(const std::string& description,
                                    const ScreensInfo& screens,
                                    const std::string& expectedScaling,
                                    const char* endDescription = nullptr);

    void testTwoScreensScaleFactorScaling(const std::string& description,
                                          const ScreensInfo& screens,
                                          const std::string& expectedScaling,
                                          const char* endDescription = nullptr);

    void testGetLogMessages();

    void testSetScaleFactorWhenEnvVarAlreadySet(const std::string& expectedLastLogLine);

    void testSetScaleFactorWhenEnvVarAlreadySet(
        const std::vector<std::string>& expectedLastLogLines);

    void checkLogMessages(ScaleFactorManager& manager, double dpi, const std::string& lastLogLine);

    void checkLogMessages(ScaleFactorManager& manager,
                          double dpi,
                          const std::vector<std::string>& lastLogLines);

    static ScreensInfo create640x480desk();

    static ScreensInfo createFullHDdesk(double dpi, double ratio = 1.);

    static ScreensInfo create4Kdesk(double dpi, double ratio = 1.);

    static ScreensInfo createFullHDand4Kdesk(double dpi, double ratio = 1.);

    std::string floatVal(const std::string& val, const std::string& decimals);

    static constexpr auto scaleEnvironmentVariableName{"QT_SCALE_FACTOR"};
    static constexpr auto scaleScreensEnvironmentVariableName{"QT_SCREEN_SCALE_FACTORS"};
    static constexpr auto autoScaleEnvironmentVariableName{"QT_AUTO_SCREEN_SCALE_FACTOR"};

#ifdef WIN32
    void unsetenv(std::string name);
    void setenv(std::string name, std::string value, int /*overwrite*/);
#endif

    void unsetEnvironmentVariables();

    void configureAsUbuntu20();

    void configureAsDeepin20();

    void configureAsWindows10();

private:
    std::string createSectionName(const std::string& description,
                                  const char* endDescription,
                                  const char* defaultEndDescription);

    static std::vector<std::string> toStdStringVector(const QVector<QString>& param);

    OsType mOsType;
    QString mOsName;
    QString mDesktopName;
    const std::string mDecimalSeparator;
};

#endif // SCALEFACTORMANAGERTESTFIXTURE_H
