#pragma once
#include <vector>
#include <memory>
#include <string>

struct ScreenInfo
{
    std::string name;
    int availableWidthPixels;
    int availableHeightPixels;
    double dotsPerInch;
    double devicePixelRatio;

    std::string toString() const
    {
        return name + ", " + std::to_string(availableWidthPixels) + ", " + std::to_string(availableHeightPixels)
                + ", " + std::to_string(dotsPerInch) + ", " + std::to_string(devicePixelRatio);
    }
};

enum class OsType{LINUX=0, WIN, MACOS};

using ScreensInfo = std::vector<ScreenInfo>;

class ScaleFactorManager
{
public:
    ScaleFactorManager(OsType osType);
    ScaleFactorManager(OsType osType, ScreensInfo screensInfo, std::string osName);
    void setScaleFactorEnvironmentVariable();
    std::vector<std::string> getLogMessages() const;

private:
    OsType mOsType;
    ScreensInfo mScreensInfo;
    mutable std::vector<std::string> logMessages;
    std::vector<double> calculatedScales;

    bool checkEnvirontmentVariables() const;
    void computeScales();
    double computeScaleLinux(const ScreenInfo& screenInfo) const;
    double getDpiOnLinux() const;
    ScreensInfo createScreensInfo();
};
